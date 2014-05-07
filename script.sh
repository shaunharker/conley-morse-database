#!/bin/bash
#Active comments for SGE
#$ -V
#$ -cwd
#$ -j y
#$ -S /bin/bash
#$ -pe orte 896

/opt/openmpi/bin/mpiexec --mca mpi_preconnect_all 1 -np $NSLOTS -x LD_LIBRARY_PATH ./Conley_Morse_Database $ENV_MODELDIR
