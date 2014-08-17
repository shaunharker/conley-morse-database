# makefile for distrib project                                             
CC := mpicxx
CXX := mpicxx
SOFTWARE := ../../../../
BOOST := $(SOFTWARE)
CXXFLAGS := -std=c++11 -I$(SOFTWARE)/include -I ../../../include -ftemplate-depth-2048
LDFLAGS := -L$(SOFTWARE)/lib
LDLIBS := -lboost_serialization -lboost_thread -lboost_system -lboost_chrono -lsdsl -ldivsufsort -ldivsufsort64
LDFLAGS += -Wl,-rpath,"$(abspath $(BOOST))/lib"
all: main

main: main.o
	$(CC) $(LDFLAGS) main.o -o $@ $(LDLIBS)
.PHONY: clean
clean:
	rm -f *.o
	rm -f main
