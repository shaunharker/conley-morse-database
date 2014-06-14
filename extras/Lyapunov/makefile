# makefile for distrib project                                             

export PREFIX:=../../
include ../../makefile.config

CXXFLAGS += -I./include

all: Lyapunov

Lyapunov: source/Lyapunov.o
	$(CC) $(LDFLAGS) source/Lyapunov.o -o $@ $(LDLIBS)
	mv Lyapunov $(MODELDIR)/computeLyapunov;

.PHONY: clean
clean:
	rm -f source/*.o
	rm -f Lyapunov
