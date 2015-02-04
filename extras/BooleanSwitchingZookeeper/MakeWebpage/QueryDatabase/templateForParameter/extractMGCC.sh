#!/bin/bash


#/bin/ls -al > output.txt

../../../../postprocessing/./BSTest $1 $2 $3 $4

name=`basename $3 .txt`

cp ../${name}.txt equations.txt
tar cvfz  ${name}_MGCC_$4.tgz walls.txt variables.txt parametersInequalities.txt outEdges.txt equations.txt
