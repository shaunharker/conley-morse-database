<?php
//
$dirdata=$_POST['dir'];

$dirdata="../../../../postprocessing/".$dirdata;

$output=array();
// Read the Graphviz file for the parameter graph
$fileContent=file_get_contents( $dirdata."/parameterGraphForaGivenMGCC.gv" ) or die ("could not open file");
$output["network"] = $fileContent;
// Read the parameter inequalities corresponding to each node of the parameter graph
$fileContent=file_get_contents( $dirdata."/parameterGraphInequalities.txt" ) or die ("could not open file2");
//$output["info"]=json_decode(str_replace(PHP_EOL,"",$fileContent));
// adding colors to the parameter inequalities

$jsonData=json_decode(str_replace(PHP_EOL,"",$fileContent));

$newinequalities=array();

// will have just one array
foreach ($jsonData->data as $mydata) {
  // loop over the parameters
  foreach($mydata as $key => $val ) {
    $ineqstmp=array();
    // split the inequalities with separator ';'
    $ineqArray=explode(";",$val);
    // Loop over the different inequalities' line
    foreach ($ineqArray as $ineq) {
      // split the string with '<=', if more than one need to rewrite the inequalities
      $tmp = explode("<=", $ineq);
      if ( count ($tmp) == 3 ) {
        array_push($ineqstmp,$tmp[0] . "<=" . $tmp[1]);
        array_push($ineqstmp,$tmp[1] . "<=" . $tmp[2]);
      }
      if ( count($tmp) == 2 ) {
        array_push($ineqstmp,$ineq);
      }
    }
    $newinequalities[$key]=$ineqstmp;
  }
}


// intersection between the different set of inequalities to get the common set
$inter=reset($newinequalities);
foreach ( $newinequalities as $key => $val ) { 
	$inter=array_intersect($inter,$val);
}

$myarray=array();

foreach ( $newinequalities as $key => $myvaluearray ) {
  $mystr="";
  foreach ( $myvaluearray as $val ) {
    if ( in_array($val,$inter) ) {
      $mystr .= " <font color=\"blue\"> $val </font><br><br> ";
    } else {
      $mystr .= "$val <br><br>";
    }
  }
  $myarray[$key] = $mystr;
}

$output["info"]=$myarray;

//
echo json_encode($output);
//


?>
