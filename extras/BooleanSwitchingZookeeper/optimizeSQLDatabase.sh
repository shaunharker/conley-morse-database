#!/bin/bash 

# script to create index in two tables in the SQL database.
# This will speed up (a lot) the query 

# argument the path of the database
modeldir=$1
cp optimizeSQL_backup.txt optimizeSQL.txt
sqlite3 $modeldir/database.db < optimizeSQL.txt

 

