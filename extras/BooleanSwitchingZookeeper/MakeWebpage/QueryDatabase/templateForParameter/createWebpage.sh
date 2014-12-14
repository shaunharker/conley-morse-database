#!/bin/bash 

# This script will output indexSQL.html
# with the radio buttons to perform queries 
# on the SQL database through the PHP file checkradio-form.php 

# argument is the path to the source code of the BooleanSwitching
sourcedir="$1"

file="$2/indexSQL.html"

rm $file
printf "<html> \n <head> \n <title> $2 </title> \n </head> \n <body> \n" > $file

# for the radio buttons
printf "<form action=\"checkradio-form.php\" method=\"post\" target=\"MGframe\">\n" >> $file
printf "<p>Find the Morse graph that contains at least one Morse set with the following properties :</p>\n" >> $file
printf "<table border=\"1\">\n" >> $file
# columns title
printf "<tr> \n <th> Yes </th> \n <th> No </th> \n <th> Either </th> \n</tr> \n" >> $file
# go through the radio buttons
# read the annotations.h file and extract the Morse Set conditions
counter=0
while read line; do 
# Warning : To find the condition used in the Boolean Switching Code
# we scan through the file AnnotationConditions.h
# For the conditions, we assume we have something of the form : 
#   #define C**** 
    string=`echo $line | grep '^#define C' | cut -d'"' -f2`
    if [ -n "$string" ]; then
    	counter=$(( $counter+1 )) # >> $file
	    printf "<tr> \n" >> $file
	    printf "<td> <input type=\"radio\" name=\"radio["$counter"]\" " >> $file
	    # value convention (colon separated) ->  status : symbol
	    printf "value=\"" >> $file
	    tmp=`echo $string | cut -d':' -f1`
	    value="Y:"$tmp
	    printf %q "$value" | sed "s/\\\\//g" >> $file
	    printf "\" /> </td> \n" >> $file
	    #
	    printf "<td> <input type=\"radio\" name=\"radio["$counter"]\" " >> $file
			printf "value=\"" >> $file
	    tmp=`echo $string | cut -d':' -f1`
	    value="N:"$tmp
	    printf %q "$value" | sed "s/\\\\//g" >> $file
	    printf "\" /> </td> \n" >> $file
	    #
	    printf "<td> <input type=\"radio\" name=\"radio["$counter"]\" " >> $file
			# no need for symbol since nothing to check in SQL
			printf "value=\"E" >> $file
	    printf "\" checked /> </td> \n" >> $file
	    #
	    printf "<td> " >> $file 
			printf %q "$string" | sed "s/\\\\//g" >> $file
	    printf " </td>\n" >> $file
	    printf "</tr>" >> $file
	  fi
done < $sourcedir/AnnotationConditions.h
printf "</table>\n" >> $file
printf "<br>" >> $file
printf "<input type=\"submit\" name=\"formSubmit\" value=\"Submit\" />\n" >> $file
printf "</form>\n" >> $file

# put the iframe
printf "<iframe name=\"MGframe\" width=\"1200\" height=\"900\"> </iframe>" >> $file

printf "</body> \n </html>" >> $file

