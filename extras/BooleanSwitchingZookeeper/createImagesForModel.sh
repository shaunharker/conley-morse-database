#!/bin/bash 

modeldir=$1
dest=$2

subdirs=`find "$modeldir" -mindepth 1 -maxdepth 1 -type d -name '*' | sed 's:^\./::'`

cd MakeWebpage
for i in $subdirs
do
  ./createImages.sh $i $dest
echo $i
done
cd ..
# make an extra copy of the network for the model
 
