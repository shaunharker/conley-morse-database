<?php

/*
PHP code to query the SQL Database of the model for a parameter
(permutation)
It returns an HTML table to update the webpage indexSQL.html
The HTML/PHP codes are in the parameter subdirectory
*/

try {

// retrieve the name of the parameter subdirectory from the path
$actual_link = "http://$_SERVER[HTTP_HOST]$_SERVER[REQUEST_URI]";
$pieces=explode("/",$actual_link);
$parameterName=$pieces[count($pieces)-3];

echo $parameterName;

	// Open database
	$file_db = new PDO('sqlite:../../database.db');
	$file_db->setAttribute(PDO::ATTR_ERRMODE,
	PDO::ERRMODE_EXCEPTION);

	// SQL query returns : name of permutations, # morsegraph found, min. percentage, max. percentage, sum of percentage

	$sqlquery = "select distinct MORSEGRAPHS.MORSEGRAPHFILEID from MORSEGRAPHS ";
	$sqlquery .= " INNER JOIN PERMUTATIONS ON MORSEGRAPHS.PERMUTATIONID=PERMUTATIONS.PERMUTATIONID ";
	$sqlquery .= " AND PERMUTATIONS.PERMUTATIONDIR=";
	$sqlquery .= "\"".$parameterName."\""; 
	$sqlquery .= " INNER JOIN MORSESETS ON MORSESETS.MORSEGRAPHID=MORSEGRAPHS.MORSEGRAPHID ";
	$sqlquery .= " AND MORSESETS.PERMUTATIONID=PERMUTATIONS.PERMUTATIONID ";
	$condition="";
	$firstpass=false;
	$firstcondition=false;
	foreach ( $_POST['radio'] as $message ) {
		$pos = strpos($message,':');
		if ( $pos !== false ) {
			// retieve the radio button status and symbol
			$status = substr($message, 0, $pos);
			$symbol = substr($message, $pos+1);
			if ( $status == "Y" ) {
				if ( $firstpass == false or $firstcondition == true ) {
					$condition .= " AND ";
				}
				$condition .= "\"".$symbol."\"=1";
				$firstpass = true;
				$firstcondition = true;
			}
			if ( $status == "N" ) {
				if ( $firstpass == false or $firstcondition == true ) {
					$condition .= " AND ";
				}
				$condition .= "\"".$symbol."\"=0";
				$firstpass = true;
				$firstcondition = true;
			}
		}
	}

	$sqlquery .= $condition;

	$results = $file_db->query($sqlquery) or die ('Query failed');

	echo "<table border=\"1\" >";

	$counter=0;
	foreach ( $results as $m ) {
	  if ( $counter == 0 ) {
	    echo "<tr>";
	  }
	  $MGfileindex=$m[MORSEGRAPHFILEID];
	  echo "<td> <img src=\"graphs/MGCC$MGfileindex.png\" usemap=\"#MGCC{$MGfileindex}\" width=\"200\" > </td>";
	  $counter = $counter + 1;
	  if ( $counter==5 ) {
	    echo "</tr>";
	    $counter = 0;
	  }
	  // Introduce the "map" objects
	  include ( "graphs/MGCC{$MGfileindex}-cmapx.html" );
	}
	echo "</table>";
	
	$file_db -> close();
}
catch (PDOException $e) {
	echo $e->getMessage();
}

?>
