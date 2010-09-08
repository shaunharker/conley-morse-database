# makefile for CMDP project                                                                       

CXX := mpicxx
HOMEDIR := /home/sharker
CAPDDIR := $(HOMEDIR)/capd
CHOMPDIR := $(HOMEDIR)/chomp-rutgers
BOOSTDIR := $(HOMEDIR)/boost_1_42_0
LIBS = -L$(BOOSTDIR)/stage/lib/ -lboost_serialization
CXXFLAGS := -O3 -m64 -Wall -I./include/ -I$(CAPDDIR)/include -I$(BOOSTDIR) -I$(CHOMPDIR)/include -Wno-deprecated


# The last part is because the conley2 boost library is not up to date and I have
# a new one installed TEMPORARILY.                                                
# -m64 means we want to use 64 bit code                                                            
# -O3 tells the compiler to try to use its most advanced optimizations                             
# -Wall tells the compiler to turn on all warnings                                                 
# -pg is for the profiler. REMOVE it for a final build.                                            

CXX_STANDALONE := $(CXX) $(CXXFLAGS) $(LIBS)

VPATH = ./source/data_structures:./source/program:./source/program/jobs:./source/distributed:./include/data_structures:./include/program:./include/program/jobs:./include/distributed:./test/

all: Conley_Morse_Database

DATABASE_OBJECTS = Conley_Morse_Database.o Message.o Communicator.o Worker.o Coordinator.o

Conley_Morse_Database: $(DATABASE_OBJECTS)
	$(CXX_STANDALONE) $(DATABASE_OBJECTS) -o Conley_Morse_Database

Test_Morse_Graph: Test_Morse_Graph.o 
	$(CXX_STANDALONE) Test_Morse_Graph.o -o $@

Test_Clutching_Graph: Test_Clutching_Graph.o
	$(CXX_STANDALONE) Test_Clutching_Graph.o -o $@
# Conley_Morse_Database.o: Message.o Communicator.o 

Message.o: Message.h

Communicator.o: Communicator.h

# Cleanup                                                                                          

.PHONY: clean
clean:
	rm *.o
