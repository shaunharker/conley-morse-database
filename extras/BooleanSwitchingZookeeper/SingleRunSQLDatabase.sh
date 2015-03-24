#!/bin/bash

datadir=$1
casename=$2

# Remove existing SQL Database
if [ -f $datadir/database.db ] 
then
	rm $datadir/database.db
fi

# case index (used for models with multiple cases)
n=1

# Generate the database and store into its appropriate directory
#
cp makeSQLDatabaseZooForAGivenModel/main $datadir/mainSQL 
(cd $datadir; ./mainSQL $datadir/database.mdb $datadir/ $casename.txt ./ $n $casename)
(cd $datadir; rm mainSQL)

# optimize the database 
sqlite3 $datadir/database.db < optimizeSQL.txt

# remove previous Zoo directory
rm -rf $datadir/Zoo

mkdir $datadir/Zoo
mkdir $datadir/Zoo/graphs

mv $datadir/MGCC* $datadir/Zoo/graphs/
mv $datadir/HASSE* $datadir/Zoo/graphs/

# Add the html/php files for the webpage (Shaun's stuff)
cp -r template/*  $datadir/Zoo/
( cd $datadir/Zoo ; ./COMPUTE graphs )



