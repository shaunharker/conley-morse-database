#!/bin/bash
# Installer for conley-morse-database

CUR_DIR=`pwd`
PREFIX=/usr/local
if [ $# -ge 1 ]; then
    # The user supplied an argument
    PREFIX=${1}
    # Get absolute path name of install directory
    mkdir -p "${PREFIX}" 2> /dev/null
    cd "${PREFIX}" > /dev/null 2>&1
    if [ $? != 0 ] ; then
        echo "ERROR: '${PREFIX}' does not exist nor could be created."
        echo "Please choose another directory."
        exit 1
    else
        PREFIX=`pwd -P`
    fi
fi
echo "Software will be installed in '${PREFIX}'"
cd ${CUR_DIR}
if [ ! -d ${PREFIX}/include ]; then
    mkdir ${PREFIX}/include
fi
if [ ! -d ${PREFIX}/lib ]; then
    mkdir ${PREFIX}/lib
fi
if [ ! -d ${PREFIX}/bin ]; then
    mkdir ${PREFIX}/bin
fi

if [ "$(uname)" == "Darwin" ]; then
    # Mac OS X platform        
    echo "Detected Mac OS X platform."
    sudo ./bin/install-mac.sh ${PREFIX}
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    # Linux platform
    echo "Detected Linux platform."
    ./bin/install-linux.sh ${PREFIX}
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    # Windows NT platform
    echo "Detected Windows MINGW platform"
    ./bin/install-windows.sh ${PREFIX}
fi
