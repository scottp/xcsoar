#
# The XCSoar test suite.
#

topdir = ..

SRC = $(topdir)/Common/Source
HDR = $(topdir)/Common/Header

include $(topdir)/build/sdl.mk
include $(topdir)/build/targets.mk
include $(topdir)/build/debug.mk
include $(topdir)/build/verbose.mk
include $(topdir)/build/compile.mk

######## paths

INCLUDES := $(TARGET_INCLUDES) -I$(HDR) -I$(SRC)

######## compiler flags

CPPFLAGS := $(INCLUDES) $(TARGET_CPPFLAGS)
CPPFLAGS	+= -DFLARM_AVERAGE

CXXFLAGS	:=$(OPTIMIZE) -fno-exceptions $(PROFILE)
CFLAGS		:=$(OPTIMIZE) $(PROFILE)

include $(topdir)/build/warnings.mk

####### linker configuration

LDFLAGS = $(TARGET_LDFLAGS) $(PROFILE)
LDLIBS = $(TARGET_LDLIBS)

include $(topdir)/build/sdl.mk

.phony: 

BUILDTESTS=\
	t/01_test_tap.exe

<<<<<<< HEAD:test/Makefile
# NOTE: For now this is hard coded, but should take it or called from main Makefile
SRC=../Common/Source
HDR=../Common/Header
INCLUDES        := -I$(HDR)/mingw32compat -I$(HDR) -I$(SRC)
CPPFLAGS        := $(INCLUDES)
=======
all: $(BUILDTESTS) RunWayPointParser-$(TARGET)$(TARGET_EXEEXT)
>>>>>>> 8bba777c0859dfba80203ab83c08f5da33819f49:test/Makefile

test: .phony $(BUILDTESTS)
	# prove
	perl tools/testall.pl t/*

# TODO generalise to rules (how to do a rule for alternative directories)
t/01_test_tap.exe: src/01_test_tap.c
	gcc $(CPPFLAGS) -o $@ $<

t/20_xmlParser.exe: src/20_xmlParser.c $(HDR)/xmlParser.h $(SRC)/xmlParser.cpp
	gcc $(CPPFLAGS) -o $@ src/20_xmlParser.c $(SRC)/xmlParser.cpp

RUN_WAY_POINT_PARSER_OBJS = \
	$(SRC)/Waypointparser.o \
	$(SRC)/WayPointList.o \
	$(SRC)/Math/Earth.o \
	$(SRC)/Math/FastMath.o \
	$(SRC)/Math/Geometry.o \
	$(SRC)/UtilsText.o \
	$(SRC)/zzip-$(TARGET).a \
	$(SRC)/compat-$(TARGET).a \
	$(SRC)/LocalPath.o \
	src/RunWayPointParser.o
RunWayPointParser-$(TARGET)$(TARGET_EXEEXT): $(RUN_WAY_POINT_PARSER_OBJS:.o=-$(TARGET).o)
	@$(NQ)echo "  LINK    $@"
	$(Q)$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

include $(topdir)/build/zzip.mk
include $(topdir)/build/compat.mk

clean:
	rm -f $(BUILDTESTS) RunWayPointParser-$(TARGET)$(TARGET_EXEEXT)
	rm -f $(RUN_WAY_POINT_PARSER_OBJS:.o=-$(TARGET).o)
