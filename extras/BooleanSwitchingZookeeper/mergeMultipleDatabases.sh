#!/bin/bash 

# We want to merge two databases from two files.
# file1 : database1.db
# file2 : database2.db
# file2 is the destination file 

datadir=$1
databaseout=$2

filesin=`find "$datadir" -mindepth 1 -maxdepth 1 -type d -name "*" | sed 's:^\./::'` 

ofile=mergeSQLScriptnew.txt

rm ${ofile}
#rm $datadir/database.db

for i in $filesin 
do
#date
#echo $i

# part 1 
if [ ! -f $databaseout ] 
then
  cp $i/database.db  $databaseout
  echo "ATTACH \""$databaseout"\" as db2;" > $ofile 
else
# part 2
  echo "ATTACH \""$i/database.db"\" as db1;" >> $ofile

# we removed the check if the entry already exist or not. much faster without it

  echo "INSERT INTO db2.permutations select * FROM db1.permutations;" >> $ofile

  echo "INSERT INTO db2.morsegraphs select * FROM db1.morsegraphs;" >> $ofile

  echo "INSERT INTO db2.morsesets select * FROM db1.morsesets;" >> $ofile

 
#  echo "INSERT INTO db2.permutations select * FROM db1.permutations t2 WHERE NOT EXISTS(SELECT permutationid from db2.permutations t1 WHERE t1.permutationid=t2.permutationid);" >> $ofile

#  echo "INSERT INTO db2.morsegraphs select * FROM db1.morsegraphs t2 WHERE NOT EXISTS(SELECT permutationid from db2.morsegraphs t1 WHERE t1.permutationid=t2.permutationid);" >> $ofile

#  echo "INSERT INTO db2.morsesets select * FROM db1.morsesets t2 WHERE NOT EXISTS(SELECT permutationid from db2.morsesets t1 WHERE t1.permutationid=t2.permutationid);" >> $ofile

  echo "DETACH DATABASE \"db1\";" >> $ofile

fi

#sed "s|file1|$i/database.db|" mergeSQLCommands.txt > tmp.txt
#sed -i "s|file2|$databaseout|" tmp.txt

# if file2 does not exist then we create it from file1 (simple copy)
#if [ ! -f $databaseout ] 
#then
#  cp $i/database.db  $databaseout
#else
# Merge the database into file2 through sqlite
#sqlite3 < tmp.txt
#fi

done

echo "starting now"
date
sqlite3 < $ofile
echo "done"
date


