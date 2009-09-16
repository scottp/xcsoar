.phony: 

BUILDTESTS=\
	t/01_test_tap.exe

# NOTE: For now this is hard coded, but should take it or called from main Makefile
SRC=../Common/Source
HDR=../Common/Header
INCLUDES        := -I$(HDR)/mingw32compat -I$(HDR) -I$(SRC)
CPPFLAGS        := $(INCLUDES)

test: .phony $(BUILDTESTS)
	# prove
	perl tools/testall.pl t/*

# TODO generalise to rules (how to do a rule for alternative directories)
t/01_test_tap.exe: src/01_test_tap.c
	gcc $(CPPFLAGS) -o $@ $<

t/20_xmlParser.exe: src/20_xmlParser.c $(HDR)/xmlParser.h $(SRC)/xmlParser.cpp
	gcc $(CPPFLAGS) -o $@ src/20_xmlParser.c $(SRC)/xmlParser.cpp

clean:
	rm $(BUILDTESTS)

