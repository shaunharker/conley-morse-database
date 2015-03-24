#!/bin/bash 


resultdir=$1

sourcedir="/share/data/goullet/ConleyMorseDatabase/conley-morse-database/examples/BooleanSwitching/"

# find the first level, i.e. dimension
dimensions=`find "$resultdir" -mindepth 1 -maxdepth 1 -type d -name "5D"`

for dim in $dimensions
do
echo $dim
  # find the different models for a given dimension
  models=`find "$dim" -mindepth 1 -maxdepth 1 -type d -name "*"`
  for model in $models
  do 
    echo $model

./MakeDatabaseForModel.sh $model $sourcedir

  done
done

