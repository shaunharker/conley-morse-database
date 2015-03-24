#!/bin/bash 
#
# Script to generate the SQL database for a given model
# All the permutations are being read 
# The HTML/PHP code are also created to allow SQL Query
# on the model
#
#
# Path of the model 
# This directory contains all the permutations
modeldir=$1
#
# Path of the source code of the Boolean switching network
sourcedir=$2
#
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

  makeSQLDatabaseZooForAGivenModel/main $i/database.mdb $i/ $networkfile $modeldir $counter $permutationname

# post process the data for the given parameter/permutation
  # move the graphviz/text file to the permutation subdirectory
  rm -rf graphs
  mkdir graphs
  mv *.txt graphs/
  mv *.gv graphs/
  rm -rf $i/Zoo
  mkdir $i/Zoo
  mv graphs $i/Zoo  
  # create the png files used for the websites
  cd MakeWebpage
  ./createImages.sh $i
  # copy HTML/PHP for SQL query on a given parameter
  ./createQueryParameterWebpage.sh $i/Zoo $sourcedir
  cd ..
  # copy HTML files for regular Zoo query
  cp -r template/* $i/Zoo 
  # convert the files in graphs to png
  $i/Zoo/COMPUTE $i 
  
# remove unnecessary files 
#  rm *.txt 
#  rm *.gv

# not the best 
  mv $i/network.png $modeldir/

  counter=$((counter+1))
done

cd MakeWebpage
./createQueryModelWebpage.sh $modeldir $sourcedir 
cd ..

