#!/bin/bash
#
# From a given parameter directory containing the CMDB database.
# we generate a set of images .png used for the webpages
# The network description text file is converted to a graphviz file and png file
#
#
# Path where the database is
databasedir=$1

# extract the name of the directory
# Remark: The name of the directory is used as a prefix for some of the
# file names
dirname=`echo $databasedir | rev | cut -d'/' -f1 | rev`

# create the png for the network equations
		a2ps --no-header --border=no "$databasedir/$dirname.txt" -o test.ps
	  ps2eps --rotate=+ test.ps
	  convert -density 300 -quality 100 test.eps test.png
	  mv test.png "$databasedir/equations.png"
	  rm test.*
# convert the txt file into a graphviz file and then .png
	  ./convertTXTtoGV.sh "$databasedir/${dirname}.txt"
	  dot "$databasedir/${dirname}.gv" -Tpng -o"$databasedir/network.png"
