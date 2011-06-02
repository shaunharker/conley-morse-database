#!/bin/bash
#PBS -l nodes=15:ppn=8

export LD_LIBRARY_PATH="/usr/local/lib:/opt/lib:/opt/open-mpi/tcp-gnu41/lib"
mpiexec  /home/sharker/conley-morse-database/Conley_Morse_Database
