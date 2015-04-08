<?php

$MGCCnumber = $_POST['MGCCnumber'];

if ( is_numeric($MGCCnumber) ) {

// Run the code 
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

// run the code 
$cmd = "./seeParameterGraphMGCC.sh  " . $mytempdir . " ". dirname($dir);
$cmd .= "/database.mdb ";
$cmd .= dirname($dir);
$cmd .= "/ " . $name . ".txt";
$cmd .= " " . $MGCCnumber;
$cmd .= " 2>&1";

shell_exec($cmd) or die("could not run the code");

// return the temporary directory name
echo $mytempdir;

}

?>
