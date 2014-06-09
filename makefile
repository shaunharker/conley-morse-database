############################################
# makefile for Conley-Morse Database project
############################################
# 
COMPUTE_MORSE_SETS := no
COMPUTE_CONTINUATION := no
COMPUTE_CONLEY_INDEX := yes
# Optional Libraries: CAPD Library, SDSL Library
USE_CAPD := no
USE_SDSL := yes
# Monotonic subdivision property: set to yes
# if map satisfied F(A) < F(B) whenever A < B
# and better partial orders can be obtained
MONOTONICSUBDIVISION := no
# END OF ADVANCED OPTIONS
###########################################
#
ifeq ($(USE_CAPD),no)
USE_BOOST_INTERVAL := yes
endif 
#
include makefile.config

ifeq ($(USE_SDSL),yes)
	CXXFLAGS += -DUSE_SDSL
endif

ifeq ($(MONOTONICSUBDIVISION),yes)
	CXXFLAGS += -D MONOTONICSUBDIVISIONPROPERTY
endif

ifeq ($(USE_BOOST_INTERVAL),yes)
	CXXFLAGS += -D USE_BOOST_INTERVAL
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

Conley_Morse_Database: $(DATABASE)
	$(CC) $(LDFLAGS) $(DATABASE) -o $@ $(LDLIBS)

SingleCMG:
	$(MAKE) -C ./extras/SingleCMG clean
	$(MAKE) -C ./extras/SingleCMG MODELDIR=../../$(MODELDIR)

Lyapunov:
	$(MAKE) -C ./extras/Lyapunov clean
	$(MAKE) -C ./extras/Lyapunov MODELDIR=../../$(MODELDIR)


# Cleanup
 .PHONY: clean
clean:
	find ./build -name "*.o" -delete
	find ./build -name "*.so" -delete
	$(MAKE) -C ./extras/SingleCMG clean
	$(MAKE) -C ./extras/Lyapunov clean

# Create build directories
.PHONY: build-dirs
dirs:
	mkdir build
	mkdir build/program
