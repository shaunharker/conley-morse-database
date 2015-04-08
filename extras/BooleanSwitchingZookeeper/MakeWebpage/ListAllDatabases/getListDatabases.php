<?php
	/* set out document type to text/javascript instead of text/html */
	header("Content-type: text/javascript");
	// Extract the path of all the databases
	// We expect the following structure
	//
	//--|
	//	|- 2D-|
	//	|			| - model1 -|
	//	|			|						|- permutation1
	// 	|			|						|- permutation2
	// 	|			|
	//	|			| - model2 -|
	// 	|									|- permutation1
	//	|
	//	|- 3D-|
	//				| - model1 -|
	//										|- permutation1 
	
//	$dim = $_GET["dim"];

	$output=array();

	// loop through the directory dimensions of the form *D
	//foreach ( glob("?D") as $dimdir ) {
		// for a given dimension, we will store the model
		//$modeldata = array();
		// loop through each individual models 
//		foreach ( glob("$dim/?D*") as $modeldir ) {
//			$modelname = explode("/",$modeldir);
//			$modelname = $modelname[1];		
			// for a given model, we will store the permutation	
//			$permutationdata = array();
			// loop through all the permutations
			foreach ( glob("?D*") as $permdir) {
				$permutationame = explode("/",$permdir);
				$permutationame = $permutationame[0];
//				$permutationdata["$permutationame"] = $permdir;
				$output["$permutationame"] = $permdir;
			}
		//	$modeldata["$modelname"] = $permutationdata;
			//$output["$modelname"] = $permutationdata;
//		}
		//$output["$dimdir"] = $modeldata;
	//}

	echo json_encode($output);

?>
