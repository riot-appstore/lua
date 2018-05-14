
RM= rm -f
LD=CC
ARFLAGS = rcs

CWARNS= \
	-Wall \
	-pedantic \
	-Wextra \
	-Wshadow \
	-Wsign-compare \
	-Wundef \
	-Wwrite-strings \
	-Wredundant-decls \
	-Wdisabled-optimization \
	-Waggregate-return \
	-Wdouble-promotion \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wstrict-prototypes \
	-Wc++-compat \
	-Wold-style-definition
	#-Wno-aggressive-loop-optimizations   # not accepted by clang \
	#-Wlogical-op   # not accepted by clang \
	# the next warnings generate too much noise, so they are disabled
	# -Wconversion  -Wno-sign-conversion \
	# -Wsign-conversion \
	# -Wconversion \
	# -Wstrict-overflow=2 \
	# -Wformat=2 \
	# -Wcast-qual \

# -DEXTERNMEMCHECK -DHARDSTACKTESTS -DHARDMEMTESTS -DTRACEMEM='"tempmem"'
# -g -DLUA_USER_H='"ltests.h"'
# -pg -malign-double
# -DLUA_USE_CTYPE -DLUA_USE_APICHECK
# (in clang, '-ftrapv' for runtime checks of integer overflows)
# -fsanitize=undefined -ftrapv
TESTFLAGS= -DDEBUG_OVERRIDE_SIZES

OFLAGS?=-O2
CFLAGS?= -std=c99 $(OFLAGS) $(CWARNS)
DEPFLAGS= -MM -MP -MQ $@ -MQ $*.o -MQ $*-dbg.o
LIBS?= -lm -ldl -lreadline
LDFLAGS?=$(LIBS)

ifeq ($(LUA_32BITS),true)
CFLAGS+= -DLUA_32BITS
endif

# rwildcard
#	Use this function to recursively search for all files with a certain
#	extension
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

ALL_C_FILES=$(call rwildcard,,*.c)
C_FILES=$(filter-out lua.c,$(ALL_C_FILES))
D_FILES=$(call rwildcard,,*.d)
O_FILES=$(call rwildcard,,*.o)
GCH_FILES=$(call rwildcard,,*.gch)

NEEDED_OBJECTS=$(C_FILES:.c=.o)
DBG_OBJECTS=$(C_FILES:.c=-dbg.o)

all: lua liblua.a lua-dbg liblua-dbg.a

# ---------------------------  cleaning -------------------------------------- #

.PHONY: clean
clean: $(foreach f,$(O_FILES),$(f)-clean) \
		$(foreach f,$(GCH_FILES),$(f)-clean) \
		liblua.a-clean lua-clean liblua-dbg.a-clean lua-dbg-clean

PHONY: depclean
depclean: $(foreach dfile,$(D_FILES),$(dfile)-clean)

.PHONY: allclean
allclean: clean depclean

%-clean:
	$(RM) $*

# ----------------------------- DEPENDENCIES --------------------------------- #

# If we are only cleaning then ignore the dependencies
ifneq ($(MAKECMDGOALS),depclean)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),allclean)
-include $(C_FILES:.c=.d)
endif
endif
endif

%.d: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) $< >$@

# --------------------------- Output targets --------------------------------- #

liblua.a: $(NEEDED_OBJECTS)

liblua-dbg.a: $(DBG_OBJECTS)

lua: lua.o liblua.a

lua-dbg: lua-dbg.o liblua-dbg.a

# --------------------------- Implicit rules --------------------------------- #

%.a:
	@echo $(MSG_ARCHIVING)
	$(AR) $(ARFLAGS) $@ $^

%-dbg.o: %.c
	# Override optimization options
	$(CC) $(CFLAGS) -O0 -g -dA -DLUA_DEBUG -DDEBUG_OVERRIDE_SIZES -c $< -o $@
