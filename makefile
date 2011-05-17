# makefile for CMDP project                                                                       

CXX := mpicxx
HOMEDIR := ..
CAPDDIR := $(HOMEDIR)/capd
CHOMPDIR := $(HOMEDIR)/chomp-rutgers
BOOSTDIR := $(HOMEDIR)/boost_1_42_0
LIBDIR = -L$(BOOSTDIR)/stage/lib/ -L$(CHOMPDIR)/lib/ -L/usr/X11/lib/
LIBS = $(LIBDIR) -lboost_serialization -lchomp-rutgers -lX11
CXXFLAGS := -O3 -m64 -Wall -I./include/ -I$(CAPDDIR)/include -I$(CHOMPDIR)/include -I$(INCLUDES) -Wno-deprecated -I$(BOOSTDIR) -ggdb


# The last part is because the conley2 boost library is not up to date and I have
# a new one installed TEMPORARILY.                                                
# -m64 means we want to use 64 bit code                                                            
# -O3 tells the compiler to try to use its most advanced optimizations                             
# -Wall tells the compiler to turn on all warnings                                                 
# -pg is for the profiler. REMOVE it for a final build.                                            

CXX_STANDALONE := $(CXX) $(CXXFLAGS) $(LIBS)

VPATH = ./source/data_structures:./source/program:./source/program/jobs:./source/distributed:./include/data_structures:./include/program:./include/program/jobs:./include/distributed:./test/:./source/tools/:./include/tools/:./source/tools/lodepng/:./include/tools/lodepng/

all: Conley_Morse_Database TESTS

.PHONY: TESTS
TESTS: Test_Morse_Graph Test_Single_Box_Job

DATABASE_OBJECTS = Conley_Morse_Database.o Message.o Communicator.o Worker.o Coordinator.o picture.o lodepng.o

Conley_Morse_Database: $(DATABASE_OBJECTS)
	$(CXX_STANDALONE) $(DATABASE_OBJECTS) -o Conley_Morse_Database $(LIBS)

Test_Morse_Graph: Test_Morse_Graph.o
	$(CXX_STANDALONE) Test_Morse_Graph.o -o $@ $(LIBS)

TestConleyMorseGraph: TestConleyMorseGraph.o picture.o lodepng.o
	$(CXX_STANDALONE) TestConleyMorseGraph.o picture.o lodepng.o -o $@ $(LIBS)

TestBipartiteClutching: TestBipartiteClutching.o picture.o lodepng.o
	$(CXX_STANDALONE) TestBipartiteClutching.o picture.o lodepng.o -o $@ $(LIBS)

Test_Single_Box_Job: Test_Single_Box_Job.o
	$(CXX_STANDALONE) Test_Single_Box_Job.o picture.o lodepng.o -o $@ $(LIBS)

Test_Clutching_Graph: Test_Clutching_Graph.o
	$(CXX_STANDALONE) Test_Clutching_Graph.o -o $@ $(LIBS)
# Conley_Morse_Database.o: Message.o Communicator.o                                                                

Message.o: Message.h

Communicator.o: Communicator.h

picture.o: picture.h

lodepng.o: lodepng.h

# Cleanup                                                                                                          

.PHONY: clean
clean:
	rm *.o
