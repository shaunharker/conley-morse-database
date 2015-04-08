
try to use absolute path

Each run/case directory should contain the following files : 
config.xml, the network file, database.mdb, database.raw

!!!! By default, it is assumed that the name of the directory is the
name of the case (i.e the prefix of the network file)

A) For a single run/case

  1) To create the images for the network and equations
     SingleRunCreateImages.sh PATH
     where PATH is the path containing database.mdb
  2) To create the SQL database and graphviz files for the Hasse,MGCC graphs
     SingleRunSQLDatabase.sh PATH NAME
     PATH : directory containing database.mdb
     NAME : prefix of the network file
  3) To create the SQL Webpages
     SingleRunSWLWebpages.sh PATH SRC
     PATH : directory containing database.mdb
     SRC : directory of the boolean switching code (most likely: examples/BooleanSwitching)


     
B) For a model containing multiple cases

Structure : Model/case*/

We use temporary directory to postprocess the data to avoid to delete/change
original output data

1)  
constructMultipleDatabases.sh : takes 2 arguments
- the path of the model where all the subcase computations were done (CMD
  output )
- the path where we want to put some of the postprocessing data (will be temporary)
 
This will submit to different node the construction of the individual database for each case.
There will be also text/graphviz files for MGCC and HASSE information
We are using a temporary directory (2nd argument) because a lot of files will be created.

2)
postProcessing.sh
Once constructMultipleDatabases.sh is done, run the script postProcessing.sh
It takes two arguments 
- 1st argument should the same as the second argument of constructMultipleDatabases.sh
- 2nd argument is the path to the Switching code used to run the cases

This will : 
- put together the MGCC/HASSE files into Zoo/graphs for each case
- put the html/php files for each case

3) 
createImagesForModel.sh 
It takes one argument, i.e. the path of the model data (typically the same as 1st argument of constructMultipleDatabases.sh)

This convert the text files case information such as network (will be redundant), equations into an image for the website
This is not done in the temporary directory because there is a very small chance that something goes wrong here.
We will make also an extra copy of the image network for the model.

4) 
mergePostprocessedData.sh
It takes 2 arguments ( the same as constructMultipleDatabases.sh )

If everything went smoothly in 1) and 2), this script will move the postprocessed data (temporary directory) into the model data

