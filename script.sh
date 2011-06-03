#!/bin/bash
#PBS -l nodes=15:ppn=8

cd $PBS_O_WORKDIR
mpiexec  ./Conley_Morse_Database
