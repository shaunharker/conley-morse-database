#############################################
# makefile for Conley-Morse Database project#
#############################################

#############################################
# basic options                             #
#############################################
COMPUTE_MORSE_SETS := yes
COMPUTE_CONTINUATION := yes
COMPUTE_CONLEY_INDEX := yes

#############################################
# advanced options                          #
#############################################
USE_CAPD := no

##############################
### DO NOT EDIT BELOW THIS ###
##############################

include makefile.config

.PHONY: all
all: Conley_Morse_Database

# pattern rule for compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# List of targets
all: Conley_Morse_Database

#conley-morse-database target
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

# SingleCMG target
SingleCMG:
	$(MAKE) -C ./extras/SingleCMG clean
	$(MAKE) -C ./extras/SingleCMG MODELDIR=../../$(MODELDIR)

# Lyapunov target
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
