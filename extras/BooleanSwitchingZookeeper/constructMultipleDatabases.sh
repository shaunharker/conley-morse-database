#!/bin/bash

modeldir=$1
resultsdir=$2

subdirs=`find "$modeldir" -mindepth 1 -maxdepth 1 -type d -name '*' | sed 's:^\./::'`

rm -rf $resultsdir
mkdir $resultsdir

n=1

mypath=`pwd`

for i in $subdirs
do
  # name of the run
  name=`echo $i | rev | cut -d'/' -f1 | rev`
  # create a subrdirectory to store the database for the given parameter
  mkdir $resultsdir/$name
  # 
  # Generate the database and store into its appropriate directory
  #
  cp makeSQLDatabaseZooForAGivenModel/main $resultsdir/$name/
  # modify the script to submit into SGE
  cp scriptSQL.sh $resultsdir/$name/
  # remove existing SQL database
  rm $resultsdir/$name/database.db
  # Change the script appropriately  
  sed -i "s|mypath|$i/|g" $resultsdir/$name/scriptSQL.sh
  sed -i "s/counter/$n/" $resultsdir/$name/scriptSQL.sh
  sed -i "s|permutationname|$name|" $resultsdir/$name/scriptSQL.sh
  sed -i "s|network|$name.txt|" $resultsdir/$name/scriptSQL.sh
  # Submit the job
  cd $resultsdir/$name 
  qsub scriptSQL.sh
  cd ../..
n=$((n+1))
done

