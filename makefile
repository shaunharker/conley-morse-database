# makefile for CMDP project 
CXX := mpicxx
HOMEDIR := ..
CHOMPDIR := $(HOMEDIR)/chomp-rutgers
DELEGATORDIR := $(HOMEDIR)/cluster-delegator
BOOSTDIR := /usr/local/
LIBDIR = -L/usr/local/lib -L$(CHOMPDIR)/lib/ -L/usr/X11/lib/
#LIBS = $(LIBDIR) -Wl,-Bstatic -lboost_serialization -Wl,-Bdynamic -lchomp-rutgers -lX11
LIBS = $(LIBDIR) -lboost_serialization -lchomp-rutgers -lX11
INCLUDES := -I$(CHOMPDIR)/include -I$(DELEGATORDIR)/include -I$(BOOSTDIR)/include -I./include/
CXXFLAGS := -O3 -m64 -Wall $(INCLUDES) -Wno-deprecated -ggdb

CXX_STANDALONE := $(CXX) $(CXXFLAGS) $(LIBS)

VPATH = ./source/data_structures:./source/program:./source/program/jobs:./include/data_structures:./include/program:./include/program/jobs:./test/:./source/tools/:./include/tools/:./source/tools/lodepng/:./include/tools/lodepng/

all: Conley_Morse_Database SingleCMG PostProcessDatabase

.PHONY: TESTS

DATABASE_OBJECTS = Conley_Morse_Database.o ConleyProcess.o MorseProcess.o Database.o picture.o lodepng.o

Conley_Morse_Database: $(DATABASE_OBJECTS)
	$(CXX_STANDALONE) $(DATABASE_OBJECTS) -o Conley_Morse_Database $(LIBS)

POSTPROCESSOBJECTS = PostProcessDatabase.o Database.o
PostProcessDatabase: $(POSTPROCESSOBJECTS)
	$(CXX_STANDALONE) $(POSTPROCESSOBJECTS) -o PostProcessDatabase $(LIBS)

SingleCMG: SingleCMG.o picture.o lodepng.o
	$(CXX_STANDALONE) SingleCMG.o picture.o lodepng.o -o $@ $(LIBS)

picture.o: picture.h

lodepng.o: lodepng.h

# Cleanup                                                                                                          

.PHONY: clean
clean:
	rm *.o
