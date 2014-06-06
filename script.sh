#!/bin/bash
#Active comments for SGE
#$ -V
#$ -cwd
#$ -j y
#$ -S /bin/bash
#$ -pe orte 896

cd $ENV_MODELDIR
/opt/openmpi/bin/mpiexec --mca mpi_preconnect_mpi 1 -np $NSLOTS -x LD_LIBRARY_PATH ./Conley_Morse_Database ./
