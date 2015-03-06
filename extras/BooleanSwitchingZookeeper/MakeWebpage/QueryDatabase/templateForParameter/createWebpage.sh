#!/bin/bash 

# This script will output indexSQL.html
# with the radio buttons to perform queries 
# on the SQL database through the PHP file checkradio-form.php 

# argument is the path to the source code of the BooleanSwitching
# In order to retrieve the labels used for the morse sets
sourcedir="$1"

# The file should be in Zoo/ subdirectory of a given case
file="$2/indexSQL.html"

rm $file
printf "<html> \n <head> \n" > $file
printf "  <title> Zoo </title> \n" >> $file 
# javascript version 
printf "  <script type='text/javascript' src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.1/jquery.min.js'></script>\n" >> $file
printf "  <style>\n" >> $file
printf "  .divsql{ \n position: relative; \n left: 20px; \n width: 550px; \n height: 250px; \n display:inline-block; \n }\n" >> $file
printf "  .divpostprocessing{ \n position: relative; \n left: 30px; \n top: -100px; \n display:inline-block; \n }\n" >> $file
printf "  </style>\n" >> $file
printf "  </head> \n" >> $file
printf "  <body> \n" >> $file
# Javascript functions 
printf "  <script> \n\n" >> $file
#
printf "//given a mgcc extract the morse sets information into a tgz file and trigger a download for it\n" >> $file
printf "function downloadMorseSetInformation(mgcc,incc) {\n" >> $file
printf "  var filename='';\n" >> $file
printf "  request = $.ajax({\n" >> $file
printf "    url:  \"extractMorseSetMGCC.php\",\n" >> $file
printf "    type: \"POST\",\n" >> $file
printf "    data: {"MGCCnumber": mgcc, "INCCnumber": incc},\n" >> $file
printf "    success : function (d) {\n" >> $file 
printf "	filename = d;\n" >> $file
printf "	window.location = d;\n" >> $file
printf "    }\n" >> $file
printf "  });\n" >> $file 
printf "}\n\n" >> $file
#
printf "//given a mgcc, open a new window with the parameter graph\n" >> $file
printf "function seeParameterGraph(mgcc) {\n" >> $file
printf "  request = $.ajax({\n" >> $file
printf "    url:  \"seeParameterGraphMGCC.php\",\n" >> $file
printf "    type: \"POST\",\n" >> $file
printf "    data: {MGCCnumber: mgcc},\n" >> $file
printf "    success : function (d) {\n" >> $file
printf "	// pass the random directory name\n" >> $file
printf "	var tmp = d.split(\"/\");\n" >> $file
printf "	var ran = tmp[tmp.length-1];\n" >> $file
printf "	window.open(\"indexParameterGraph.html\"+\"?\"+String(ran));\n" >> $file
printf "    }\n" >> $file
printf "  });\n" >> $file
printf "}\n\n" >> $file
#
printf "//given a mgcc, create files on parameter graph to be downloaded\n" >> $file 
printf "function downloadParameterGraph(mgcc) {\n" >> $file
printf "var filename = '';\n" >> $file
printf "  request = $.ajax({\n" >> $file
printf "    url:  \"extractParameterGraphMGCC.php\",\n" >> $file
printf "    type: \"POST\",\n" >> $file
printf "    data: {MGCCnumber: mgcc},\n" >> $file
printf "    success : function (d) {\n" >> $file
printf "	filename = d;\n" >> $file
printf "	window.location = d;\n" >> $file
printf "    }\n" >> $file
printf "  });\n" >> $file
printf "}\n\n" >> $file
#
printf "function selectPostprocessing(selectedObj) {\n" >> $file
printf "  // get the MGCC number from the input text\n" >> $file
printf "  var mgcc = document.getElementById('MGCCnumber').value;\n" >> $file
printf "  switch(selectedObj.selectedIndex) {\n" >> $file
printf "    case 1: // download morse sets information\n" >> $file
printf "      var incc = document.getElementById('INCCnumber').value;\n" >> $file
printf "      downloadMorseSetInformation(mgcc,incc);\n" >> $file
printf "      break;\n" >> $file
printf "    case 2: // See parameter graph\n" >> $file
printf "      seeParameterGraph(mgcc);\n" >> $file
printf "      break;\n" >> $file
printf "    case 3: // Download parameter graph files\n" >> $file
printf "      downloadParameterGraph(mgcc);\n" >> $file
printf "      break;\n" >> $file
printf "  }\n" >> $file
printf "}\n\n" >> $file
printf "</script>\n" >> $file
#
#
# for the radio buttons
printf "<div class=\"divsql\">\n " >> $file
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
#
printf "</table>\n" >> $file
printf "<br>" >> $file
printf "<input type=\"submit\" name=\"formSubmit\" value=\"Submit\" />\n" >> $file
printf "</form>\n" >> $file
printf "</div>" >> $file
#
printf "<div class=\"divpostprocessing\">\n" >> $file
printf "  <table>\n" >> $file
printf "    <tr> Postprocessing : (INCC ignored for parameter graph) </tr>\n" >> $file
printf "    <tr>\n" >> $file
printf "        <td> MGCC:</td>\n" >> $file
printf "        <td> <input type=\"text\" id=\"MGCCnumber\"></td>\n" >> $file
printf "        <td> INCC:</td>\n" >> $file
printf "        <td> <input type=\"text\" id=\"INCCnumber\"></td>\n" >> $file
printf "        <td> <select onchange=\"selectPostprocessing(this);\" id=\"postprocessing\">\n" >> $file
printf "           <option value=\"0\"> -- Select -- </option>\n" >> $file
printf "           <option value=\"1\"> Download the Morse sets information </option>\n" >> $file
printf "           <option value=\"2\"> See the Parameter Graph </option>\n" >> $file
printf "           <option value=\"3\"> Download the Parameter Graph </option>\n" >> $file
printf "        </select></td>\n" >> $file
printf "    </tr>\n" >> $file
printf "  </table>\n" >> $file
printf "</div>\n\n" >> $file
#
# put the iframe
printf "<iframe name=\"MGframe\" width=\"1200\" height=\"900\"> </iframe>\n" >> $file

printf "</body> \n </html>" >> $file

