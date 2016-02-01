/*
 * Copyright (c) 2016, 8Kdata Technology S.L.
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose,
 * without fee, and without a written agreement is hereby granted, provided that the above copyright notice and this
 * paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL 8Kdata BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF 8Kdata HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 8Kdata SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS,
 * AND 8Kdata HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 */


#include "pgj.h"


PG_MODULE_MAGIC;


static JNIEnv *jniEnv;
static JavaVM *jvm;


static void handleSigterm(SIGNAL_ARGS) {
    int oldErrno;
    
    elog(LOG, "SIGTERM called");
    
    oldErrno = errno;
    if(MyProc)
        SetLatch(&MyProc->procLatch);
    errno = oldErrno;
    
    // TODO: This method is currently not being called
}

static void destroyJVM() {
    elog(LOG, "Destroying JVM");
    (*jvm)->DestroyJavaVM(jvm);
    proc_exit(0);   // TODO: should not be needed, if handleSigterm is called
}

static void startJVM() {
    JavaVMInitArgs vmArgs;
    JavaVMOption *vmOptions;
    jint createVMResult;
    jclass class;
    jmethodID mainMethod;
    jstring mainMethodArg;
    
    /* VM arguments and options */
    vmArgs.version = JNI_VERSION_1_6;
    vmArgs.ignoreUnrecognized = JNI_FALSE;
    vmArgs.nOptions = 1;
    vmOptions = (JavaVMOption*) palloc(sizeof(JavaVMOption));
    sprintf(vmOptions->optionString, "-Djava.class.path=%s", PGJ_JAVA_ARTIFACT_PATH);
    vmArgs.options = vmOptions;
    
    /* Create and set destroy handler */
    createVMResult = JNI_CreateJavaVM(&jvm, (void**) &jniEnv, &vmArgs); 
    if(JNI_OK != createVMResult) {
        elog(ERROR, "Could not create JVM. Error code: %d", createVMResult);
        return;
    }
    on_proc_exit(destroyJVM, 0);
    pfree(vmOptions);
    elog(LOG, "JVM started");
    
    /* Run the Java code */
    class = (*jniEnv)->FindClass(jniEnv, PGJ_JAVA_MAIN_CLASS);
    if(NULL == class) {
        (*jniEnv)->ExceptionDescribe(jniEnv);
        elog(ERROR, "Could not find Main class");
        return;
    }
    
    mainMethod = (*jniEnv)->GetStaticMethodID(jniEnv, class, "main", "(Ljava/lang/String;)V");
    if(NULL == mainMethod) {
        elog(ERROR, "Could not find main method");
        (*jniEnv)->ExceptionDescribe(jniEnv);
        return;
    }
    
    mainMethodArg = (*jniEnv)->NewStringUTF(jniEnv, "pgj");
    (*jniEnv)->CallStaticVoidMethod(jniEnv, class, mainMethod, mainMethodArg);
    if((*jniEnv)->ExceptionCheck(jniEnv)) {
        elog(ERROR, "Main method failed");
        (*jniEnv)->ExceptionDescribe(jniEnv);
        (*jniEnv)->ExceptionClear(jniEnv);
        return;
    }
    
    elog(LOG, "Main method successfully run(ing)");
}

/*
 * Entry point
 */
static void pgj(Datum mainArg) {
    int wakeupConditions;
    
    /* Manage signal processing */
    pqsignal(SIGTERM, handleSigterm);
    BackgroundWorkerUnblockSignals();
    
    /* Start JVM */
    startJVM();
    
    /* Exit when signaled or postmaster dies */
    ResetLatch(&MyProc->procLatch);
    wakeupConditions = WaitLatch(
            &MyProc->procLatch,
            WL_LATCH_SET | WL_POSTMASTER_DEATH,
            0
    );
    
    if(wakeupConditions & WL_POSTMASTER_DEATH) {
        proc_exit(1);
    } else if(wakeupConditions & WL_LATCH_SET) {
        proc_exit(0);
    }
}

/*
 * Library load point
 */
void _PG_init(void) {
    BackgroundWorker worker;
    
    MemSet(&worker, 0, sizeof(BackgroundWorker));
    snprintf(worker.bgw_name, BGW_MAXLEN, "pgj");
    worker.bgw_main = pgj;
    worker.bgw_main_arg = (Datum) 0;
    worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
    worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
    worker.bgw_restart_time = 1;
    worker.bgw_notify_pid = 0;
    
    RegisterBackgroundWorker(&worker);
}
