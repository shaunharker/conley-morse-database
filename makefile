# makefile for CMDP project                                                                       

CXX := mpicxx
HOMEDIR := ..
CAPDDIR := $(HOMEDIR)/capd
CHOMPDIR := $(HOMEDIR)/chomp-rutgers
BOOSTDIR := $(HOMEDIR)/boost_1_42_0
LIBDIR = -L$(BOOSTDIR)/stage/lib/ -L$(CHOMPDIR)/lib/
LIBS = $(LIBDIR) -lboost_serialization -lchomp-rutgers
CXXFLAGS := -O3 -m64 -Wall -I./include/ -I$(CAPDDIR)/include -I$(CHOMPDIR)/include -Wno-deprecated -I$(BOOSTDIR) 


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
	$(CXX_STANDALONE) $(DATABASE_OBJECTS) -o Conley_Morse_Database $(LIBS)

Test_Morse_Graph: Test_Morse_Graph.o 
	$(CXX_STANDALONE) Test_Morse_Graph.o -o $@ $(LIBS)

Test_Single_Box_Job: Test_Single_Box_Job.o 
	$(CXX_STANDALONE) Test_Single_Box_Job.o -o $@ $(LIBS)

Test_Clutching_Graph: Test_Clutching_Graph.o
	$(CXX_STANDALONE) Test_Clutching_Graph.o -o $@ $(LIBS)
# Conley_Morse_Database.o: Message.o Communicator.o 

Message.o: Message.h

Communicator.o: Communicator.h

# Cleanup                                                                                          

.PHONY: clean
clean:
	rm *.o
