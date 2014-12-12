/*
PHP code to query the SQL Database of the model across all the parameters
(permutations)
It returns an HTML table to update the webpage indexSQL.html
*/

<?php

try {
	// Open database (create it if it does not exist)
	$file_db = new PDO('sqlite:database.db');
	$file_db->setAttribute(PDO::ATTR_ERRMODE,
	PDO::ERRMODE_EXCEPTION);

	// SQL query returns : name of permutations, # morsegraph found, min. percentage, max. percentage, sum of percentage
	$sqlquery="select PERMUTATIONS.PERMUTATIONDIR,COUNT(MORSEGRAPHS.PERCENTAGE), "
	$sqlquery .= "ROUND(100.0*MIN(MORSEGRAPHS.PERCENTAGE),2)";
	$sqlquery .= ",ROUND(100.0*MAX(MORSEGRAPHS.PERCENTAGE),2)";
	$sqlquery .= ",ROUND(100.0*SUM(MORSEGRAPHS.PERCENTAGE),2)";
	$sqlquery .= " FROM MORSEGRAPHS";
	$sqlquery .= " INNER JOIN PERMUTATIONS ON PERMUTATIONS.PERMUTATIONID=MORSEGRAPHS.PERMUTATIONID";
	$sqlquery .= " INNER JOIN MORSESETS ON MORSEGRAPHS.PERMUTATIONID=MORSESETS.PERMUTATIONID ";
	$sqlquery .= " AND MORSEGRAPHS.MORSEGRAPHID=MORSESETS.MORSEGRAPHID ";

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

	$sqlquery .= " GROUP BY MORSEGRAPHS.PERMUTATIONID;";

	echo "Query summary:";
	echo "<table border=\"1\" >";
	echo "<tr>";
	echo "<td> Permutation name </td>";
	echo "<td> # morsegraph found </td>";
	echo "<td> Min. percentage found </td>";
	echo "<td> Max. percentage found </td>";
	echo "<td> sum. of morsegraph percentage found </td>";
	echo "</tr>";
	$results = $file_db->query($sqlquery) or die ('Query failed');
	foreach ( $results as $m ) {
		echo "<tr>";
		echo "<td> $m[0] </td>";
		echo "<td> $m[1] </td>";
		echo "<td> $m[2] </td>";
		echo "<td> $m[3] </td>";
		echo "<td> $m[4] </td>";
		echo "<td> $m[5] </td>";
		echo "</tr>";
	}
	echo "</table>";
	$file_db -> close();
}
catch (PDOException $e) {
	echo $e->getMessage();
}

?>
