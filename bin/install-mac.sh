#!/bin/bash

OSVERSION=`sw_vers -productVersion | grep -o "\..*\." | sed 's/\.//g'` 

if [ "`echo "${OSVERSION} < 9" | bc`" == "1" ]; then
  echo Versions of Mac OS X less than 10.9 are known to have issues.
  echo Please update your OS to use this installer.
  exit 1
fi
 
PREFIX=$1
cd ..

# Relax permissions of /usr/local
chown -R $USER:admin /usr/local 2> error.log || (echo "The installer would like to change the permissions of /usr/local for Homebrew." && echo "This requires a password." && echo "Note: if you do not want to use Homebrew then quit this installer and install the prerequisites by hand." && sudo -k chown -R $USER:admin /usr/local)
rm error.log

# Homebrew
echo
echo ==\> Homebrew
echo Checking for Homebrew.
if [ ! -f /usr/local/bin/brew ]; then
  echo Not found.
  ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)" || exit 1
  brew doctor
  echo Homebrew is now installed.
else
  echo Found. Checking for updates.
  brew update
  brew upgrade
fi

echo
echo ==\> CImg
echo Checking for CImg.
if [ ! -f /usr/local/include/CImg.h ]; then
  brew install CImg || exit 1
  echo CImg now installed.
else
  echo CImg already installed.
fi

# Boost

echo
echo ==\> Boost
echo Checking for Boost.
# Trash unbrewed copies.
if [ ! -L /usr/local/include/boost ]; then
  if [ -d /usr/local/include/boost ]; then
    echo Detected unbrewed copy of Boost. Trashing it.
  fi
  rm -rf /usr/local/include/boost
  rm -f /usr/local/lib/libboost*
fi

if [ ! -d /usr/local/include/boost ]; then
  echo Boost needs to be installed.
  if [ ! -d /usr/local/Cellar/boost ]; then
    echo Brewing Boost with Homebrew.
    brew install boost --c++11 || exit 1
  else
    echo Linking Boost from Homebrew Cellar.
    (brew unlink boost && brew link boost) || exit 1
  fi
  echo Boost now installed.
else
  echo Boost already installed.
fi

# For some reason Boost is not creating a couple needed links
if [ ! -f /usr/local/lib/libboost_thread.a ]; then
  ln -s /usr/local/lib/libboost_thread-mt.a \
        /usr/local/lib/libboost_thread.a
  ln -s	/usr/local/lib/libboost_thread-mt.dylib \
        /usr/local/lib/libboost_thread.dylib
fi

# Open-MPI
echo
echo ==\> OpenMPI
echo Checking for OpenMPI.
if [ ! -f /usr/local/bin/mpicxx ]; then
  echo Not found. Installing OpenMPI.
  brew install open-mpi || exit 1
  echo Open-MPI now installed.
else
  echo Open-MPI already installed.
fi

# CMake
echo
echo ==\> cmake
echo Checking for cmake.
if [ ! -f /usr/local/bin/cmake ]; then
  echo Not found. Installing cmake.
  brew install cmake || exit 1
  echo CMake now installed.
else
  echo CMake already installed.
fi

# SDSL
echo
echo ==\> SDSL
echo Checking for SDSL.
if [ ! -f /usr/local/lib/libsdsl.a ] || [ ! -d /usr/local/include/sdsl ]; then
  echo Not found.
  if [ ! -d sdsl-lite ]; then
    echo Cloning SDSL repository.
    git clone https://github.com/simongog/sdsl-lite.git || exit 1
    cd sdsl-lite
  else 
    echo Found sdsl repository. Updating.
    cd sdsl-lite
    git pull origin master || exit 1
  fi
  echo Running SDSL installer.
  ./install.sh /usr/local || exit 1
  cd ..
  echo SDSL now installed.
else
  echo SDSL already installed.
fi

# cluster-delegator
echo
echo ==\> cluster-delegator
echo Checking for cluster-delegator.
if [ ! -d /usr/local/include/delegator ] || [ ! -f /usr/local/include/boost/serialization/unordered_set.hpp ]; then
  echo Not found.
  if [ ! -d cluster-delegator ]; then
    echo Cloning cluster-delegator repository 
    git clone https://github.com/sharker81/cluster-delegator.git || exit 1
    cd cluster-delegator
  else
    echo Found cluster-delegator repository
    cd cluster-delegator || exit 1
    git pull origin master || exit 1
  fi
  echo Running installer script
  ./install.sh || exit 1
  cd ..
  echo cluster-delegator installed.
else
  echo cluster-delegator already installed.
fi
    
# X11 (XQuartz)
echo
echo ==\> X11
echo Checking for X11 \(XQuartz\).
if [ ! -d /opt/X11 ]; then
  echo Installing XQuartz from
  echo http://xquartz.macosforge.org/landing/
  curl http://xquartz-dl.macosforge.org/SL/XQuartz-2.7.6.dmg -o ~/Downloads/XQuartz-2.7.6.dmg || (echo "Download failed. Please install XQuartz manually" && exit 1)
  hdiutil attach ~/Downloads/XQuartz-2.7.6.dmg
  sudo installer -pkg /Volumes/XQuartz-2.7.6/XQuartz.pkg -target /
  hdiutil detach /Volumes/XQuartz-2.7.6
else
  echo X11 already installed.
fi

# CHomP
echo
echo ==\> CHomP
echo Checking for CHomP.
if [ ! -d /usr/local/include/chomp ]; then
  echo CHomP not found.
  if [ ! -d CHomP ]; then
    echo Cloning CHomP repository
    git clone https://github.com/sharker81/CHomP.git || exit 1
    cd CHomP
  else
    echo Found CHomP repository
    cd CHomP || exit 1
    git pull origin master || exit 1
  fi
  echo Installing CHomP
  ./install.sh || exit 1
  cd .. || exit 1
  echo CHomP installed.
else
  echo CHomP already installed.
fi

# GraphViz
echo
echo ==\> GraphViz
echo Checking for GraphViz.
if [ "`which dot`" == "" ]; then
  echo Not found.
  echo Installing GraphViz from www.graphviz.org
  curl http://www.graphviz.org/pub/graphviz/stable/macos/mountainlion/graphviz-2.36.0.pkg -o graphviz-2.36.0.pkg || (echo "Download failed. Please install GraphViz manually" && exit 1)
  sudo installer -pkg graphviz-2.36.0.pkg -target /
else
  echo Graphviz already installed.
fi

cd conley-morse-database
echo "PREREQ=/opt/X11 /usr/local" > makefile.dep
echo "USER_CXX_FLAGS=-stdlib=libc++" >> makefile.dep
