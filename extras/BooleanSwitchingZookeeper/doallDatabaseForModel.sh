#!/bin/bash 


resultdir=$1

# find the first level, i.e. dimension
dimensions=`find "$resultdir" -mindepth 1 -maxdepth 1 -type d -name "*D"`

for dim in $dimensions
do
echo $dim

done


#./MakeDatabaseForModel.sh /share/data/CHomP/Projects/Databases_for_the_Global_Dynamics/Databases/Switching_Networks/3D/3D_Haase_I.2.2/ /share/data/goullet/ConleyMorseDatabase/conley-morse-database/examples/BooleanSwitching/
