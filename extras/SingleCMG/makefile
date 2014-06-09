# makefile for distrib project                                             
CC := mpicxx
CXX := mpicxx
GRAPHICS := /opt/X11
SOFTWARE := ../../../
DATABASE := $(SOFTWARE)/conley-morse-database/
CXXFLAGS := -O3 -ggdb -I $(SOFTWARE)/opt/include -I $(SOFTWARE)/cluster-delegator/include -I $(SOFTWARE)/sdsl/include -I$(DATABASE)/include -I./include -I$(GRAPHICS)/include -ftemplate-depth-2048 -I$(MODELDIR)
LDFLAGS := -L $(SOFTWARE)/opt/lib -L $(SOFTWARE)/sdsl/lib -L $(GRAPHICS)/lib
LDLIBS := -lboost_serialization -lboost_thread -lboost_system -lboost_chrono -lsdsl -ldivsufsort -ldivsufsort64 -lX11

all: SingleCMG

SINGLECMG := ./source/SingleCMG.o
SingleCMG: $(SINGLECMG)
	$(CC) $(LDFLAGS) -I$(MODELDIR) $(SINGLECMG) -o $@ $(LDLIBS)
	mv SingleCMG $(MODELDIR);
	@echo "Build Finished: SingleCMG built and placed in the folder:" $(MODELDIR);

clean:
	rm -f source/*.o
