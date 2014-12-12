#!/bin/bash 

# This script will output the HTML file indexSQL.html
# with radio buttons to perform queries across all parameters
# for a given model using the PHP script 

# argument is the path to the source code of the BooleanSwitching
# This is needed to generate automatically the list of radio button necessary
sourcedir="$1"

modeldir="$2"

file="$modeldir/indexSQL.html"

if [ -e "$file" ] 
then 
	rm $file
fi 

printf "<html> \n <head> \n <title> Zoo for model </title> \n </head> \n <body> \n" > $file

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

