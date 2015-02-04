#!/bin/bash

modeldir=$1
resultsdir=$2
codepath=$3

subdirs=`find "$resultsdir" -mindepth 1 -maxdepth 1 -type d -name '*' | sed 's:^\./::'`

for i in $subdirs
do
  # name of the run
  name=`echo $i | rev | cut -d'/' -f1 | rev`

  cp -r $i/Zoo $modeldir/$name/
 
done

cp $resultsdir/database.db $modeldir/
cp $resultsdir/indexSQL.html $modeldir/
cp $resultsdir/checkradio-form.php $modeldir/
