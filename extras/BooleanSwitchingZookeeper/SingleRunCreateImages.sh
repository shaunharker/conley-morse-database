#!/bin/bash 

# Warning : Use absolute path 

# Directory containing the output : database.mdb 
datadir=$1

( cd MakeWebpage; ./createImages.sh $datadir )
