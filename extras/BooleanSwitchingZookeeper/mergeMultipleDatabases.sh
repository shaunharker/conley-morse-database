#!/bin/bash 

# We want to merge two databases from two files.
# file1 : database1.db
# file2 : database2.db
# file2 is the destination file 

datadir=$1
databaseout=$2

filesin=`find "$datadir" -mindepth 1 -maxdepth 1 -type d -name "*" | sed 's:^\./::'` 

for i in $filesin 
do

echo $i

sed "s|file1|$i/database.db|" mergeSQLCommands.txt > tmp.txt
sed -i "s|file2|$databaseout|" tmp.txt

# if file2 does not exist then we create it from file1 (simple copy)
if [ ! -f $databaseout ] 
then
  cp $i/database.db  $databaseout
else
# Merge the database into file2 through sqlite
sqlite3 < tmp.txt
fi

done
