
<?php

// Open the SQL database
$db = new SQLite3('graphs/database.sql');

// basis for the query string 
// need to retrieve the morse graph file id to fetch graphviz graphs
$sqlquery="select distinct MORSEGRAPHFILEID from morsesets where ";

$teststr='';

$testYESstr='';
$testNOstr='';
$doingquery=false;

$intersectsqlstring=" intersect select MORSEGRAPHFILEID from morsesets where ";
$countYES=0;
$istring='';

// need to keep track of the properties 'N'
$exceptsqlstring=" except select MORSEGRAPHFILEID from morsesets where ";
$estring='';
$countNO=0;
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

$results = $db->query($sqlquery) or die ('Query failed');

// Construct the Table 
echo "<table border=\"1\">";
$counter=0;
while ($row = $results->fetchArray()) {
  if ( $counter == 0 ) {
    echo "<tr>";
  }
  $MGfileindex=$row[MORSEGRAPHFILEID];
  echo "<td> <img src=\"graphs/MGCC$MGfileindex.png\" usemap=\"#MGCC{$MGfileindex}\" width=\"200\" > </td>";
  $counter = $counter + 1;
  if ( $counter==5 ) {
    echo "</tr>";
    $counter = 0;
  }
  // Introduce the "map" objects
  $doc = new DOMDocument();
  $doc->loadHTMLFile("MGCC{$MGfileindex}-cmapx.html");
  echo $doc->saveHTML();
}
echo "</table>";


?>


