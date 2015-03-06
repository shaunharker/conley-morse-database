#!/bin/bash

# in order to have the output file in the postprocessing directory
dir=../../../../postprocessing

cp $dir/extractParameterGraphMGCC $1/

( cd $1 ; ./extractParameterGraphMGCC $2 $3 $4 $5 )

( cd $1; rm extractParameterGraphMGCC )
