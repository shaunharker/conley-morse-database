# makefile for distrib project                                             
CC := mpicxx
CXX := mpicxx
GRAPHICS := /opt/X11
SOFTWARE := ../../../
DATABASE := $(SOFTWARE)/conley-morse-database/
CXXFLAGS := -O3 -ggdb -I $(SOFTWARE)/opt/include -I $(SOFTWARE)/cluster-delegator/include -I $(SOFTWARE)/sdsl/include -I$(DATABASE)/include -I./include -I$(GRAPHICS)/include -ftemplate-depth-2048 -I$(MODELDIR)
LDFLAGS := -L $(SOFTWARE)/opt/lib -L $(SOFTWARE)/sdsl/lib -L $(GRAPHICS)/lib
LDLIBS := -lboost_serialization -lboost_thread -lboost_system -lboost_chrono -lsdsl -ldivsufsort -ldivsufsort64 -lX11

all: Lyapunov

Lyapunov: source/Lyapunov.o
	$(CC) $(LDFLAGS) source/Lyapunov.o -o $@ $(LDLIBS)
	mv Lyapunov $(MODELDIR)/computeLyapunov;

.PHONY: clean
clean:
	rm -f source/*.o
	rm -f Lyapunov
