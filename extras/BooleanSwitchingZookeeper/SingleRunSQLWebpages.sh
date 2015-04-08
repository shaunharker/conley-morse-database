#!/bin/bash 

datadir=$1
srcdir=$2

# Add the SQL html/php files for the parameter webpage
MakeWebpage/QueryDatabase/templateForParameter/createWebpage.sh $srcdir $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/checkradio-form.php $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/extractMorseSetMGCC.php $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/extractMorseSetMGCC.sh $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/extractParameterGraphMGCC.php $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/extractParameterGraphMGCC.sh $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/seeParameterGraphMGCC.php $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/seeParameterGraphMGCC.sh $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/indexParameterGraph.html $datadir/Zoo/
cp MakeWebpage/QueryDatabase/templateForParameter/readGraphviz.php $datadir/Zoo/


# Need to change the path of the database for single run
sed -i.bak 's|..\/..\/database.db|..\/database.db|g' $datadir/Zoo/checkradio-form.php

