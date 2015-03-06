<?php

$MGCCnumber = $_POST['MGCCnumber'];
$INCCnumber = $_POST['INCCnumber'];

if ( is_numeric($MGCCnumber) and is_numeric($INCCnumber) ) { 

$mypostdir="../../../../postprocessing/";

# will create a temporary directory to store data
function tempdir($dir=false,$prefix='php') {
    $tempfile=tempnam("../../../../postprocessing",'');
    if (file_exists($tempfile)) { unlink($tempfile); }
    mkdir($tempfile);
    if (is_dir($tempfile)) { return $tempfile; }
}

$mytempdir=tempdir();

// Name of the threshold parameter obtained from parent directory name
$name = basename(dirname(__DIR__)); 

// get the path
$dir = __DIR__;

// filename to be downloaded ( need relative path )
$filedownloaded = $mypostdir . basename($mytempdir) . "/" . $name . "_MGCC_" . $MGCCnumber . "_INCC_" . $INCCnumber . ".tgz";

// run the code 
$cmd = "./extractMorseSetMGCC.sh  " . $mytempdir . " " . dirname($dir);
$cmd .= "/database.mdb ";
$cmd .= dirname($dir);
$cmd .= "/ " . $name . ".txt";
$cmd .= " " . $MGCCnumber . " " . $INCCnumber;
$cmd .= " 2>&1";

shell_exec($cmd) or die("could not run the code");

echo $filedownloaded;

}

?>
