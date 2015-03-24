# makefile for distrib project                                             
CC := mpicxx
CXX := mpicxx
SOFTWARE := ../../../..
CXXFLAGS := -std=c++11 -ggdb -I../../../examples/BooleanSwitching -I $(SOFTWARE)/include -I ../../../include -I../../../../cluster-delegator/include -ftemplate-depth-2048 -I$(SOFTWARE)/sqlite3/include 
LDFLAGS := -L $(SOFTWARE)/lib -Wl,-rpath,$(SOFTWARE)/lib,-L$(SOFTWARE)/sqlite3/lib
LDLIBS := -lboost_serialization -lboost_thread -lboost_system -lboost_chrono -lsdsl -ldivsufsort -ldivsufsort64 -lsqlite3

all: main

main: main.o
	$(CC) $(LDFLAGS) main.o -o $@ $(LDLIBS)
.PHONY: clean
clean:
	rm -f *.o
	rm -f main
