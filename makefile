# makefile for CMDP project 
CXX := mpicxx
HOMEDIR := ..
CHOMPDIR := $(HOMEDIR)/chomp-rutgers
BOOSTDIR := 
LIBDIR = -L/usr/local/lib -L$(CHOMPDIR)/lib/ -L/usr/X11/lib/
#LIBS = $(LIBDIR) -Wl,-Bstatic -lboost_serialization -Wl,-Bdynamic -lchomp-rutgers -lX11
LIBS = $(LIBDIR) -lboost_serialization -lchomp-rutgers -lX11

CXXFLAGS := -O3 -m64 -Wall -I./include/ -I$(CHOMPDIR)/include -I$(INCLUDES) -Wno-deprecated -I$(BOOSTDIR) -ggdb

CXX_STANDALONE := $(CXX) $(CXXFLAGS) $(LIBS)

VPATH = ./source/data_structures:./source/program:./source/program/jobs:./source/distributed:./include/data_structures:./include/program:./include/program/jobs:./include/distributed:./test/:./source/tools/:./include/tools/:./source/tools/lodepng/:./include/tools/lodepng/

all: Conley_Morse_Database PostProcessDatabase

.PHONY: TESTS
TESTS: Test_Morse_Graph Test_Single_Box_Job

DATABASE_OBJECTS = Conley_Morse_Database.o Message.o Communicator.o Worker.o Coordinator.o Database.o picture.o lodepng.o

Conley_Morse_Database: $(DATABASE_OBJECTS)
	$(CXX_STANDALONE) $(DATABASE_OBJECTS) -o Conley_Morse_Database $(LIBS)

POSTPROCESSOBJECTS = PostProcessDatabase.o Database.o
PostProcessDatabase: $(POSTPROCESSOBJECTS)
	$(CXX_STANDALONE) $(POSTPROCESSOBJECTS) -o PostProcessDatabase $(LIBS)

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
