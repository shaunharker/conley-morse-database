# makefile for CMDP project 
# directories where prerequisites can be found
HOMEDIR := ..
CHOMP := $(HOMEDIR)/chomp-rutgers
CLUSTER := $(HOMEDIR)/cluster-delegator
BOOST := /usr/local
GRAPHICS := /usr/X11

#includes
HOMEDIR := ..
INCS := -I$(CHOMP)/include
INCS += -I$(CLUSTER)/include
INCS += -I$(BOOST)/include
INCS += -I$(GRAPHICS)/include
INCS += -I./include/

#libraries
LIBS := -L$(CHOMP)/lib -lchomp-rutgers
LIBS += -L$(BOOST)/lib -lboost_serialization
LIBS += -L$(GRAPHICS)/lib -lX11

# directory to store build products
OBJDIR := ./build
SRCDIR := ./source
# Variables "make" uses for implicit rules
CC := mpicxx
CXX := mpicxx
CXXFLAGS := -O3 -m64 -ggdb $(INCS)
LDFLAGS := $(LIBS)

# pattern rule for compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# List of targets
all: Conley_Morse_Database SingleCMG PostProcessDatabase

DATABASE := ./build/program/Conley_Morse_Database.o 
DATABASE += ./build/program/ConleyProcess.o
DATABASE += ./build/program/MorseProcess.o 
DATABASE += ./build/structures/Database.o 
Conley_Morse_Database: $(DATABASE)
	$(CC) $(LDFLAGS) $(DATABASE) -o $@

POSTPROCESS := ./build/test/PostProcessDatabase.o 
POSTPROCESS += ./build/structures/Database.o
PostProcessDatabase: $(POSTPROCESS)
	$(CC) $(LDFLAGS) $(POSTPROCESS) -o $@

SINGLECMG := ./build/test/SingleCMG.o 
SINGLECMG += ./build/tools/picture.o 
SINGLECMG += ./build/tools/lodepng/lodepng.o
SingleCMG: $(SINGLECMG)
	$(CC) $(LDFLAGS) $(SINGLECMG) -o $@

# Cleanup
 .PHONY: clean
clean:
	find . -name "*.o" -delete
	rm Conley_Morse_Database
	rm SingleCMG
	rm PostProcessDatabase

