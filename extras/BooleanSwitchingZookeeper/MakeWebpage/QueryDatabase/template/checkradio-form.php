
<?php


try {
	// Open database (create it if it does not exist)
	$file_db = new PDO('sqlite:graphs/database.sql');
	$file_db->setAttribute(PDO::ATTR_ERRMODE,
     	   	               PDO::ERRMODE_EXCEPTION);

	// basis for the query string 
	// need to retrieve the morse graph file id to fetch graphviz graphs
	$sqlquery="select distinct MORSEGRAPHFILEID from morsesets where ";

	$doingquery=false;

	$intersectsqlstring=" intersect select MORSEGRAPHFILEID from morsesets where ";
	$istring='';

	// need to keep track of the properties 'N'
	$exceptsqlstring=" except select MORSEGRAPHFILEID from morsesets where ";
	$estring='';
	$NoFound=false;

	$countQuery=0;

	foreach ( $_POST['radio'] as $message ) {
		
		$pos = strpos($message,':');
		if ( $pos !== false ) {
			// retieve the radio button status and symbol
			$status = substr($message, 0, $pos);
			$symbol = substr($message, $pos+1);
	 		if ( $status == "Y" ) {
				$doingquery=true;
				if ( $countQuery == 0 ) {
					$sqlquery .= "\"".$symbol."\"=1";
				} else {	
					$istring .= $intersectsqlstring."\"".$symbol."\"=1";
				}	
				++$countQuery;
	 		}
	 		if ( $status == "N" ) {
				$doingquery=true;
				$NoFound=true;
				if ( $countQuery == 0 ) {
					$sqlquery .= "\"".$symbol."\"=0";
				} else {	
					$istring .= $intersectsqlstring."\"".$symbol."\"=0";
				}
				$estring .= $exceptsqlstring."\"".$symbol."\"=1 ";
				++$countQuery;	
	   	}
		}
	}

	if ( $doingquery == true ) {
		if ( $countQuery > 1 ) { 
			$sqlquery .= $istring;
		}
		// if selected a No, we need to append estring
		if ( NOfound == true ) {
			$sqlquery .= $estring;
		}
	} else {
	// if no basic query, we select everything
		$sqlquery="select distinct MORSEGRAPHFILEID from morsesets";
	}

	//echo "$sqlquery";

	$results = $file_db->query($sqlquery) or die ('Query failed');

	// Construct the Table 
	echo "<table border=\"1\">";
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


