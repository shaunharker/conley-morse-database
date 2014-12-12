#!/bin/bash 

# Path for the model 
modeldir="$1"
# Path of the source code of the boolean switching model 
sourcedir="$2"

cd QueryDatabase/templateForModel
cp *.php "$modeldir"

./createWebpage "$sourcedir" "$modeldir"
cd ../..
