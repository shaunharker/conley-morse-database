############################################
# makefile for Conley-Morse Database project
###########################################
#
#
CHECKIFMAPISGOOD := yes
#
COMPUTE_MORSE_SETS := yes
COMPUTE_CONTINUATION := yes
COMPUTE_CONLEY_INDEX := no
DRAW_IMAGES := yes
#
# CAPD Library 
#
USE_CAPD := yes
#
# Memory saving option : PointerGrid, SuccinctGrid 
#
HAVE_SUCCINCT := yes
PARAMETER_GRID := SuccinctGrid
PHASE_GRID := PointerGrid
#
# if modelmap has good() implemented
PARAM_SPACE_METHOD := PATCHMETHOD
MONOTONICSUBDIVISION := no
#
#
#
# END OF ADVANCED OPTIONS
###########################################
#
#ifeq ($(USE_CAPD),no)
#USE_BOOST_INTERVAL := yes
#endif 
#
include makefile.config
CXXFLAGS += -D $(PARAM_SPACE_METHOD)
CXXFLAGS += -DPARAMETER_GRID=$(PARAMETER_GRID) 
CXXFLAGS += -DPHASE_GRID=$(PHASE_GRID)

ifeq ($(HAVE_SUCCINCT),yes)
	CXXFLAGS += -DHAVE_SUCCINCT
endif

ifeq ($(MONOTONICSUBDIVISION),yes)
	CXXFLAGS += -D MONOTONICSUBDIVISIONPROPERTY
endif

ifeq ($(USE_BOOST_INTERVAL),yes)
	CXXFLAGS += -D USE_BOOST_INTERVAL
endif

ifeq ($(DRAW_IMAGES),yes)
CXXFLAGS += -D DRAW_IMAGES
endif 

.PHONY: all
all: Conley_Morse_Database

# pattern rule for compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# List of targets
all: Conley_Morse_Database

DATABASE := ./build/program/Conley_Morse_Database.o 

ifeq ($(COMPUTE_MORSE_SETS),yes)
	DATABASE += ./build/program/MorseProcess.o 
	CXXFLAGS += -D COMPUTE_MORSE_SETS
endif

ifeq ($(COMPUTE_CONTINUATION),yes)
	CXXFLAGS += -D COMPUTE_CONTINUATION
endif

ifeq ($(COMPUTE_CONLEY_INDEX),yes)
	DATABASE += ./build/program/ConleyProcess.o
	CXXFLAGS += -D COMPUTE_CONLEY_INDEX
endif

ifeq ($(CHECKIFMAPISGOOD),yes)
	CXXFLAGS += -D CHECKIFMAPISGOOD
endif

Conley_Morse_Database: $(DATABASE)
	$(CC) $(LDFLAGS) $(DATABASE) -o $@ $(LDLIBS)

COMPUTEGRAPH := ./build/program/ComputeGraph.o
ComputeGraph: $(COMPUTEGRAPH)
	$(CC) $(LDFLAGS) $(COMPUTEGRAPH) -o $@ $(LDLIBS)

GRAPHPROCESS := ./build/program/GraphProcess.o
GraphProcess: $(GRAPHPROCESS)
	$(CC) $(LDFLAGS) $(GRAPHPROCESS) -o $@ $(LDLIBS)

EXPORTXML := ./build/program/ExportXML.o
EXPORTXML += ./build/structures/Database.o
EXPORTXML += ./build/structures/XMLExporter.o

ExportXML: $(EXPORTXML)
	$(CC) $(LDFLAGS) $(EXPORTXML) -o $@ $(LDLIBS)


SINGLECMG := ./build/test/SingleCMG.o 
SingleCMG: $(SINGLECMG)
	$(CC) $(LDFLAGS) -I$(MODELDIR) $(SINGLECMG) -o $@ $(LDLIBS)
	mv SingleCMG $(MODELDIR);
	@echo "#"; 
	@echo "# Conley-Morse Database Code";
	@echo "# Single computer mode with fixed parameter values";
	@echo "# The executable file SingleCMG is the model directory:" $(MODELDIR);
	@echo "#";


# Cleanup
 .PHONY: clean
clean:
	find ./build -name "*.o" -delete
	find ./build -name "*.so" -delete
	rm -f Conley_Morse_Database
	rm -f SingleCMG

# Create build directories
.PHONY: build-dirs
dirs:
	mkdir build
	mkdir build/program
	mkdir build/structures
	mkdir build/test
	mkdir build/tools

