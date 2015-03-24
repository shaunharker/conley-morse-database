#!/bin/bash

# postprocess a  model with all its permutations 
# the query for a given permutation is not being implemented here

# path of the model directory
modeldir=$1
# path of the source code for the boolean switching network
sourcedir=$2

permutationdir=`find "$modeldir" -type d -mindepth 1 -maxdepth 1 -name "*"`

# step 1 : Create the images for each permutation
cd makeWebpage
echo "Create PNG files for every permutation directory"
for permutation in $permutationdir
do
	echo $permutation
	./createImages $permutation
done


# step 2 : Create the webpage for the queries 
#	   Note : path of the booleanswitching code is required here

./createQueryModelWebpage.sh "$modeldir" "$sourcedir"

