<?php

$MGCCnumber = $_POST['MGCCnumber'];

// Name of the threshold parameter obtained from parent directory name
$name = basename(dirname(__DIR__)); 

// get the path
$dir = __DIR__;

// filename to be downloaded
$filedownloaded = $name . "_MGCC_" . $MGCCnumber . ".tgz";

// run the code 
$cmd = "./extractMGCC.sh  " . dirname($dir);
$cmd .= "/database.mdb ";
$cmd .= dirname($dir);
$cmd .= "/ " . $name . ".txt";
$cmd .= " " . $MGCCnumber;
$cmd .= " 2>&1";

shell_exec($cmd) or die("could not run the code");


header('Content-type: application/zip');

// It will be called downloaded.pdf
$string = 'Content-Disposition: attachment; filename='.$filedownloaded;
//header('Content-Disposition: attachment; filename=$filedownloaded');
header($string);


// The PDF source is in original.pdf
readfile($filedownloaded);

$results = exec("./extractMGCCcleanup.sh");


?>
