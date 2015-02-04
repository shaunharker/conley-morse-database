#!/bin/bash 

# Path for the parameter subdirectory 
paramdir="$1"
# Path of the source code of the boolean switching model 
sourcedir="$2"

cd QueryDatabase/templateForParameter
cp checkradio-form.php "$paramdir"

./createWebpage.sh "$sourcedir" "$paramdir"
cd ../..
