#!/bin/bash 
#
#
# From the network description file 
# we output the corresponding graphviz file
#
#
sourcefile=$1
tmp=`echo ${sourcefile:0:${#sourcefile}-4}`
ofile=`echo $tmp.gv`
#
echo "starting to convert file from .txt to .gv"
# list the variables 
var=$(cat $sourcefile | cut -d':' -f1)
#
echo "digraph {" > $ofile
echo "rankdir=LR" >> $ofile
#
while read line
do
# get the left hand side
	lhs=`echo $line | cut -d':' -f1`	
# get the right hand side 
	rhs=`echo $line | cut -d':' -f2` 
#
	for v in $var 
	do
		# check v appears in the right hand side 
		tmp=`echo $rhs | grep "$v"` 
		if [ $? -eq "0" ];
			then
			# check if it is up or down regulation 
			tmp=`echo $rhs | grep "~$v"`
			if [ $? -eq "0" ];
			then
				echo $v"->"$lhs" [arrowhead=tee]" >> $ofile 
			else 
			  echo $v"->"$lhs >> $ofile
			fi
		fi		
	done
done < $sourcefile
#
echo "}" >> $ofile
echo "conversion done"
