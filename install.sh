#!/bin/bash
# Installer for conley-morse-database

# Determine if C++11 compiler is present
echo '#if __cplusplus <= 199711L' > tmp.cpp
echo '#error This library needs at least a C++11 compliant compiler' >> tmp.cpp
echo '#endif' >> tmp.cpp
echo 'int main(){return 0;}' >> tmp.cpp
g++ -std=c++11 tmp.cpp -o tmp.out 2> tmp.log || OLDCOMPILER="yes"
rm -f tmp.cpp
rm -f tmp.out
rm -f tmp.log
if [ "$OLDCOMPILER" == "yes" ]; then
  echo This software requires a C++11 compiler.
  echo Either GCC version \>= 4.7 or Clang are known to work.
  echo See INSTALL file for hints on how to resolve this problem.
  exit 1
fi

# Determine installation prefix
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
    ./bin/install-mac.sh ${PREFIX}
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    # Linux platform
    echo "Detected Linux platform."
    ./bin/install-linux.sh ${PREFIX}
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    # Windows NT platform
    echo "Detected Windows MINGW platform"
    ./bin/install-windows.sh ${PREFIX}
fi
