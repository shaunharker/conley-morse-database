#!/bin/bash

# in order to have the output file in the postprocessing directory
dir=../../../../postprocessing

cp $dir/extractMorseSetMGCC $1/

( cd $1 ; ./extractMorseSetMGCC $2 $3 $4 $5 $6 )

name=`basename $4 .txt`

cp ../${name}.txt $1/equations.txt
( cd $1 ; tar cvfz  ${name}_MGCC_$5_INCC_$6.tgz output.json equations.txt )

( cd $1 ; rm output.json equations.txt extractMorseSetMGCC )
