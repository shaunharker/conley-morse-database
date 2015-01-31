# makefile for distrib project                                             
CC := mpicxx
CXX := mpicxx
GRAPHICS := /opt/X11
SOFTWARE := ../../../
DATABASE := $(SOFTWARE)/conley-morse-database/
CXXFLAGS := -std=c++11 -O3 -ggdb -I$(SOFTWARE)/include -I $(SOFTWARE)/opt/include -I $(SOFTWARE)/cluster-delegator/include -I $(SOFTWARE)/sdsl/include -I$(DATABASE)/include -I./include -I$(GRAPHICS)/include -ftemplate-depth-2048 -I$(MODELDIR)
LDFLAGS := -L $(SOFTWARE)/opt/lib -L $(SOFTWARE)/sdsl/lib -L $(GRAPHICS)/lib -L$(SOFTWARE)/lib 

LDLIBS := -lboost_serialization -lboost_thread -lboost_system -lboost_chrono -lsdsl -ldivsufsort -ldivsufsort64 -lX11
LDLIBS += -Wl,-rpath,/share/data/goullet/ConleyMorseDatabase/lib:/opt/openmpi/lib  

all: BSTest

BSTest: main.o
	$(CC) $(LDFLAGS) main.o -o $@ $(LDLIBS)

.PHONY: clean
clean:
	rm -f main.o
	rm -f BSTest
