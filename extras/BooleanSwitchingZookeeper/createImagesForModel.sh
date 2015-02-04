#!/bin/bash 

modeldir=$1

subdirs=`find "$modeldir" -mindepth 1 -maxdepth 1 -type d -name '*' | sed 's:^\./::'`

cd MakeWebpage
for i in $subdirs
do
  ./createImages.sh $i
done
cd ..
# make an extra copy of the network for the model
 
