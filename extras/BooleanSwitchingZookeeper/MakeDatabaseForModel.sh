#!/bin/bash 

# Argument the path of the model 
# This directory contains all the permutations
modeldir=$1

# if it already exists, delete the SQL database for the model 
if [ -e $modeldir/database.db ] 
then
  rm $modeldir/database.db
fi

# find all the models subdirectories, i.e. the permutations
permutationdir=`find $modeldir -mindepth 1 -maxdepth 1 -type d -name "*"`

counter=1
for i in $permutationdir
do
  permutationname=`echo $i | rev | cut -d'/' -f1 | rev`
  networkfile=$permutationname.txt
  echo $i

  
  makeDatabaseAllSQL/main $i/database.mdb $i/ $networkfile $modeldir $counter $permutationname

# remove unnecessary files
  rm *.txt 
  rm *.gv

  counter=$((counter+1))
done

