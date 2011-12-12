# makefile for CMDP project 
# directories where prerequisites can be found
HOMEDIR := ..
CHOMP := ./
CLUSTER := $(HOMEDIR)/cluster-delegator
BOOST := /usr/local
GRAPHICS := /usr/X11
MODELDIR := ./   #user overrides this

#includes
HOMEDIR := ..
INCS := -I$(CHOMP)/include
INCS += -I$(CLUSTER)/include
INCS += -I$(BOOST)/include
INCS += -I$(GRAPHICS)/include
INCS += -I./include/
INCS += -I$(MODELDIR)

#libraries                                                                     
LINKFLAGS := -L$(CHOMP)/lib
LINKFLAGS += -L$(BOOST)/lib
LINKFLAGS += -L$(GRAPHICS)/lib

LINKLIBS := -lboost_serialization
LINKLIBS += -lX11


# directory to store build products
OBJDIR := ./build
SRCDIR := ./source

# Variables "make" uses for implicit rules
CC := mpicxx
CXX := mpicxx
CXXFLAGS := -O3 -m64 -Wall -ggdb $(INCS)
LDFLAGS := $(LINKFLAGS)
LDLIBS := $(LINKLIBS)

# pattern rule for compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# List of targets
all: Conley_Morse_Database

DATABASE := ./build/program/Conley_Morse_Database.o 
DATABASE += ./build/program/ConleyProcess.o
DATABASE += ./build/program/MorseProcess.o 
DATABASE += ./build/structures/Database.o 
Conley_Morse_Database: $(DATABASE)
	$(CC) $(LDFLAGS) $(DATABASE) -o $@ $(LDLIBS)

POSTPROCESS := ./build/test/PostProcessDatabase.o 
POSTPROCESS += ./build/structures/Database.o
PostProcessDatabase: $(POSTPROCESS)
	$(CC) $(LDFLAGS) $(POSTPROCESS) -o $@ $(LDLIBS)

SINGLECMG := ./build/test/SingleCMG.o 
SINGLECMG += ./build/tools/picture.o 
SINGLECMG += ./build/tools/lodepng/lodepng.o
SingleCMG: $(SINGLECMG)
	$(CC) $(LDFLAGS) $(SINGLECMG) -o $@ $(LDLIBS)

# Cleanup
 .PHONY: clean
clean:
	find . -name "*.o" -delete
	rm -f Conley_Morse_Database
	rm -f SingleCMG
	rm -f PostProcessDatabase

