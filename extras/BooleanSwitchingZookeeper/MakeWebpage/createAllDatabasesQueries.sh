#!/bin/bash

# path of the Boolean Switching source code
sourcedir=$1

# Path where all the databases are 
databasedir=$2

# list all the directories containing the databases
# assuming name of the form : nD****
# where n is the dimension
# here we can specify which networks we want to use by changing
# -name pattern 
listdir=`find "$databasedir" -maxdepth 1 -type d -name "[0-9]*" | sed 's:^\./::' | rev | cut -d'/' -f1 | rev `


for i in $listdir
do
	# create the html file
	cd QueryDatabase/template
	./createWebpage.sh "$sourcedir" "$i"
	cd ../..
	cp QueryDatabase/template/indexSQL.html "$databasedir/$i/Zoo"
	cp QueryDatabase/template/checkradio-form.php "$databasedir/$i/Zoo"
done



