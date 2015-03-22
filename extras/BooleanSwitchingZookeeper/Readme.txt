
try to use absolute path

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

If everything went through for 1) and 2), this script will move the postprocessed data (temporary directory) into the model data

