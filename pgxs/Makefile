
MODULE_big = pgj
PG_CPPFLAGS += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux	# TODO: don't hardcode (Linux)
SHLIB_LINK += -L$(JAVA_HOME)/jre/lib/amd64/server -ljvm				# TODO: don't hardcode (arch)
OBJS = pgj.o
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
SHAREDIR := $(shell $(PG_CONFIG) --sharedir)

PGJ_JAVA_ARTIFACT = pgj-0.10-SNAPSHOT.jar
DATA = ../java/target/${PGJ_JAVA_ARTIFACT}

$(shell echo "#define PGJ_JAVA_ARTIFACT \"$(PGJ_JAVA_ARTIFACT)\"" >> pgj_local.h)
$(shell echo "#define PGJ_SHAREDIR \"$(SHAREDIR)\"" >> pgj_local.h)
$(shell echo "#define PGJ_MODULEDIR PGJ_SHAREDIR\"/contrib\"" >> pgj_local.h)
$(shell echo "#define PGJ_JAVA_ARTIFACT_PATH PGJ_MODULEDIR\"/\"PGJ_JAVA_ARTIFACT" >> pgj_local.h)

EXTRA_CLEAN = pgj_local.h

include $(PGXS)
