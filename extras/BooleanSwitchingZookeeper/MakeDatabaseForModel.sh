#!/bin/bash 

# Argument the path of the model 
# This directory contains all the permutations
modeldir=$1

# find all the permutations subdirectories
permutationdir=`find $modeldir -type d -mindepth 1 -maxdepth 1 -name *`

../makeDatabaseAllSQL/main $modeldir 
