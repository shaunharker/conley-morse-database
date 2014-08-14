# makefile for distrib project                                             
CC := mpicxx
CXX := mpicxx
SOFTWARE := ../../../
BOOST := $(SOFTWARE)/opt/
CXXFLAGS := -ggdb -I $(SOFTWARE)/opt/include -I $(SOFTWARE)/cluster-delegator/include -I $(SOFTWARE)/sdsl/include -I ../../include -ftemplate-depth-2048
LDFLAGS := -L $(SOFTWARE)/opt/lib -L $(SOFTWARE)/sdsl/lib
LDLIBS := -lboost_serialization -lboost_thread -lboost_system -lboost_chrono -lsdsl -ldivsufsort -ldivsufsort64
LDFLAGS += -Wl,-rpath,"$(abspath $(BOOST))/lib"
all: main

main: main.o
	$(CC) $(LDFLAGS) main.o -o $@ $(LDLIBS)
.PHONY: clean
clean:
	rm -f *.o
	rm -f main
