#!/bin/bash
#
# From a directory containing the databases
# we add the javascript array containing all the databases 
# in index.html for the html list to work 
#
# The text files with the description of the equations
# are converted into .png and into a network graphviz file
# (also converted in .png)

# Path where all the databases are 
databasedir=$1

# format for the directory name nD_something where n is the dimension
listdim=`find "$databasedir" -maxdepth 1 -type d -name '[0-9]*' | sed 's:^\./::' | rev | cut -d'/' -f1 | rev | cut -d'_' -f1 | uniq | sort`

file=index.html

str='var networks = { '
for dim in $listdim
do 
	str=$str" '$dim' : [ "
	# keep only the directory names with a given dimension
	listdir=`find "$databasedir" -maxdepth 1 -type d -name "${dim}*" | sed 's:^\./::' | rev | cut -d'/' -f1 | rev `
	for i in $listdir
	do 
		str=$str" '$i', "
		# create the png for the network equations
		a2ps --no-header --border=no "$databasedir/$i/$i.txt" -o test.ps
	  ps2eps --rotate=+ test.ps
	  convert -density 300 -quality 100 test.eps test.png
	  mv test.png "$databasedir/$i/${i}_equations.png"
	  rm test.*
	  # convert the txt file into a graphviz file and then .png
	  ./convertTXTtoGV.sh "$databasedir/$i/$i.txt"
	  dot "$databasedir/$i/$i.gv" -Tpng -o"$databasedir/$i/${i}_network.png"
	done
	# remove the last comma
	str=`echo $str | rev | cut -c 2- | rev`
	str=$str" ], "
done
#remove the last comma
str=`echo $str | rev | cut -c 2- | rev`
#
str=$str" };"

cp -r ListAllDatabases/template/* "$databasedir/"

sed -i.bak "s/var\ networks.*/$str/" "$databasedir/index.html"

