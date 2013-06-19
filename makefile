# makefile for CMDP project 
COMPUTE_MORSE_SETS := no
COMPUTE_CONTINUATION := yes
COMPUTE_CONLEY_INDEX := no
USE_CAPD := no
HAVE_SUCCINCT := yes
PARAMETER_GRID := SuccinctGrid
PHASE_GRID := PointerGrid
CHECKIFMAPISGOOD := no
# if modelmap has good() implemented
PARAM_SPACE_METHOD := PATCHMETHOD
MONOTONICSUBDIVISION := no

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

WAVEPOOLCMG := ./build/test/WavePoolCMG.o
WavePoolCMG: $(WAVEPOOLCMG)
	$(CC) $(LDFLAGS) $(WAVEPOOLCMG) -o $@ $(LDLIBS)

NEWWAVEPOOLCMG := ./build/test/NewWavePoolCMG.o
NewWavePoolCMG: $(NEWWAVEPOOLCMG)
	$(CC) $(LDFLAGS) $(NEWWAVEPOOLCMG) -o $@ $(LDLIBS)


SINGLECMG := ./build/test/SingleCMG.o 
SingleCMG: $(SINGLECMG)
	$(CC) $(LDFLAGS) $(SINGLECMG) -o $@ $(LDLIBS)

RMHTEST := ./build/test/RMHTest.o
RMHTest: $(RMHTEST)
	$(CC) $(LDFLAGS) $(RMHTEST) -o $@ $(LDLIBS)

HENONCMG := ./build/test/HenonCMG.o
HenonCMG: $(HENONCMG)
	$(CC) $(LDFLAGS) $(HENONCMG) -o $@ $(LDLIBS)

NEWTONCMG := ./build/test/NewtonCMG.o
NewtonCMG: $(NEWTONCMG)
	$(CC) $(LDFLAGS) $(NEWTONCMG) -o $@ $(LDLIBS)

JUSTINCMG := ./build/test/JustinCMG.o
JustinCMG: $(JUSTINCMG)
	$(CC) $(LDFLAGS) $(JUSTINCMG) -o $@ $(LDLIBS)

ALLANCMG := ./build/test/AllanCMG.o
AllanCMG: $(ALLANCMG)
	$(CC) $(LDFLAGS) $(ALLANCMG) -o $@ $(LDLIBS)

TRAVELCMG := ./build/test/TravelCMG.o
TravelCMG: $(TRAVELCMG)
	$(CC) $(LDFLAGS) $(TRAVELCMG) -o $@ $(LDLIBS)

LVCMG := ./build/test/LVCMG.o
LVCMG: $(LVCMG)
	$(CC) $(LDFLAGS) $(LVCMG) -o $@ $(LDLIBS)

SELKOVCMG := ./build/test/SelkovCMG.o
SelkovCMG: $(SELKOVCMG)
	$(CC) $(LDFLAGS) $(SELKOVCMG) -o $@ $(LDLIBS)

SUBMAPTEST := ./build/test/SubdividedMapTest.o
SubdividedMapTest: $(SUBMAPTEST)
	$(CC) $(LDFLAGS) $(SUBMAPTEST) -o $@ $(LDLIBS)

# Cleanup
 .PHONY: clean
clean:
	find ./build -name "*.o" -delete
	find ./build -name "*.so" -delete
	rm -f Conley_Morse_Database
	rm -f SingleCMG
	rm -f ExportXML
	rm -f NewtonCMG
	rm -f AllanCMG
	rm -f JustinCMG
	rm -f SelkovCMG
	rm -f TravelCMG
	rm -f RMHTest
	rm -f SubdividedMapTest
	rm -f LVCMG

# Create build directories
.PHONY: build-dirs
dirs:
	mkdir build
	mkdir build/program
	mkdir build/structures
	mkdir build/test
	mkdir build/tools
	mkdir build/tools/lodepng
