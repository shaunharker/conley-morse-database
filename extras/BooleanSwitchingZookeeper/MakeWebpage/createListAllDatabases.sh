#!/bin/bash
#
# Script to copy the template HTML/PHP file that create a list of database available from 
# a set of subdirectories (PHP scans the main directory)
# Format expected :
#  rootDirectoryData 	-|
#			 |- 2D -|
#			 |	|- Model1 -|
#			 |	|	   |- Parameter1
#			 |	|	   |- Parameter2
#			 |	|
#			 |	|- Model2 -|
#			 |		   |- Parameter1
#			 |- 4D 
#  In the schematic above, the template files will be copies in root DirectoryData

# Path where all the databases are 
databasedir=$1

cp ListAllDatabases/template/* $1/

