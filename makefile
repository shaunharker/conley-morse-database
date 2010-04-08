# makefile for CMDP project                                                                       

CXX := mpicxx
LIBS = -L/home/sharker/boost_1_42_0/stage/lib/ -lboost_serialization
CXXFLAGS := -O3 -m64 -Wall -I./include/ -I/home/sharker/boost_1_42_0/

# The last part is because the conley2 boost library is not up to date and I have
# a new one installed TEMPORARILY.                                                
# -m64 means we want to use 64 bit code                                                            
# -O3 tells the compiler to try to use its most advanced optimizations                             
# -Wall tells the compiler to turn on all warnings                                                 
# -pg is for the profiler. REMOVE it for a final build.                                            

CXX_STANDALONE := $(CXX) $(CXXFLAGS) $(LIBS)

VPATH = ./source/data_structures:./source/program:./source/program/jobs:./source/distributed:./include/data_structures:./include/program:./include/program/jobs:./include/distributed

all: Conley_Morse_Database

DATABASE_OBJECTS = Conley_Morse_Database.o Message.o Communicator.o

Conley_Morse_Database: $(DATABASE_OBJECTS)
	$(CXX_STANDALONE) $(DATABASE_OBJECTS) -o Conley_Morse_Database

Conley_Morse_Database.o: Message.o Communicator.o

Message.o: Message.h

Communicator.o: Communicator.h

# Cleanup                                                                                          

.PHONY: clean
clean:
	rm *.o
