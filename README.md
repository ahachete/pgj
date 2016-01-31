# pgj

A JVM (Java Virtual Machine) running alongside PostgreSQL.

This is not production ready, just a proof of concept.

It only works on Linux, but it should be possible to run it on other OSes (feel free to create a PR).


## Compiling pgj

### The PostgreSQL extension (`pgxs`)

You need a suitable JDK (version 6 or above) and export its path via the `JAVA_HOME` environment variable.

    export JAVA_HOME=/path/to/jdk
    cd pgxs
    make
    sudo make install


## Running pgj

You need to make the JVM library (typically called `libjvm.so`) available in the `LIBRARY_PATH`, like:

    sudo ln -s /usr/lib/jvm/java-8-oracle/jre/lib/amd64/server/libjvm.so /usr/lib

Add the library to `postgresql.conf`'s `shared_preload_libraries`:

    shared_preload_libraries = '... other libraries if any... , pgj'		# (change requires restart)

And restart PostgreSQL.

