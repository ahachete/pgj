PG_CPPFLAGS += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux	# TODO: don't hardcode (Linux)
SHLIB_LINK += -L$(JAVA_HOME)/jre/lib/amd64/server -ljvm				# TODO: don't hardcode (arch)
MODULE_big = pgj
OBJS = pgj.o
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
