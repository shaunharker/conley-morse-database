# makefile for distrib project                                             

PREFIX:=../../
include $(PREFIX)makefile.config

all: SingleCMG

SINGLECMG := ./source/SingleCMG.o
SingleCMG: $(SINGLECMG)
	$(CC) $(LDFLAGS) -I$(MODELDIR) $(SINGLECMG) -o $@ $(LDLIBS)
	mv SingleCMG $(MODELDIR);
	@echo "Build Finished: SingleCMG built and placed in the folder:" $(MODELDIR);

clean:
	rm -f source/*.o
