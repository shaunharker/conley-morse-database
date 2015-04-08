#!/bin/bash

# in order to have the output file in the postprocessing directory
dir=../../../../postprocessing

cp $dir/extractParameterGraphMGCC $1/

( cd $1 ; ./extractParameterGraphMGCC $2 $3 $4 $5 )

name=`basename $4 .txt`

( cd $1 ; tar cvfz  ${name}_ParameterGraph_MGCC_$5.tgz parameterGraphForaGivenMGCC.gv parameterGraphInequalities.txt )

( cd $1 ; rm *.txt *.gv extractParameterGraphMGCC )
