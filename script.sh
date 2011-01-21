#!/bin/bash
#PBS -l nodes=10:ppn=8

export LD_LIBRARY_PATH="/home/sharker/boost_1_42_0/stage/lib:/opt/lib:/opt/open-mpi/tcp-gnu41/lib"
mpiexec  /home/sharker/conley-morse-database/Conley_Morse_Database

