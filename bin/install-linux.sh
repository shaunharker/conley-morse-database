# Linux Installer script for conley-morse-database
PREFIX=$1

PATH=${PREFIX}/bin:${PATH}
export PATH

cd ..
# Install Boost
echo
echo ==\> Boost
if [ ! -d ${PREFIX}/include/boost ]; then
  echo Installing Boost.
  wget http://downloads.sourceforge.net/project/boost/boost/1.56.0/boost_1_56_0.tar.gz || exit 1
  tar xvfz boost_1_56_0.tar.gz || exit 1
  cd boost_1_56_0
  ./bootstrap.sh --prefix=${PREFIX} || exit 1
  ./b2 install || exit 1
  cd ..
else
  echo Boost already installed.
fi

# Install openmpi
echo
echo ==\> OpenMPI
echo Checking for OpenMPI.
if [ "`which mpicxx`" == "" ]; then
  echo Installing OpenMPI.
  wget http://www.open-mpi.org/software/ompi/v1.8/downloads/openmpi-1.8.1.tar.gz || exit 1
  tar xvfz openmpi-1.8.1 || exit 1
  cd openmpi-1.8.1 
  ./configure --prefix=${PREFIX} || exit 1
  make all install || exit 1
  cd ..
  echo OpenMPI installed.
else
  echo OpenMPI already installed.
fi

# Install cmake
echo
echo ==\> cmake
echo Checking for cmake.
if [ "`which cmake`" == "" ]; then
  INSTALLCMAKE=yes
  echo Not found.
else
  CMAKEVERSION=`cmake --version | grep -o "[0-9]\.[0-9]"`
  if [ ` echo "$CMAKEVERSION < 3.0" | bc ` == "1" ]; then
      echo Found version too old.
      INSTALLCMAKE=yes
  fi
fi

if [ "$INSTALLCMAKE" == "yes" ]; then
  echo Installing cmake.
  wget http://www.cmake.org/files/v3.0/cmake-3.0.1.tar.gz || exit 1
  tar xvfz cmake-3.0.1.tar.gz || exit 1
  cd cmake-3.0.1 
  ./bootstrap --prefix=${PREFIX} || exit 1
  make || exit 1
  make install || exit 1
  cd ..
fi

# Install "sdsl"
echo
echo ==\> SDSL
echo Checking for SDSL.
if [ ! -f ${PREFIX}/lib/libsdsl.a ] || [ ! -d ${PREFIX}/include/sdsl ]; then
  echo Not found.
  if [ ! -d sdsl-lite ]; then
    echo Cloning SDSL repository.
    git clone https://github.com/simongog/sdsl-lite.git || exit 1
    cd sdsl-lite
  else
    echo Found SDSL repository. Updating.
    cd sdsl-lite
    git pull origin master || exit 1
  fi
  echo Running SDSL installer.
  ./install.sh ${PREFIX} || exit 1
  cd ..
  echo SDSL now installed.
else
  echo SDSL already installed.
fi

# Install CImg.h
echo
echo ==\> CImg
echo Checking for CImg.
if [ ! -f ${PREFIX}/include/CImg.h ]; then
  echo Not found. Installing CImg.
  wget http://downloads.sourceforge.net/project/cimg/CImg-1.5.9.zip || exit 1
  unzip CImg-1.5.9.zip || exit 1
  mv CImg-1.5.9/CImg.h ${PREFIX}/include/CImg.h || exit 1
  echo CImg now installed.
else
  echo CImg already installed.
fi

# Install "CHomP"
echo
echo ==\> CHomP
echo Checking for CHomP.
if [ ! -d ${PREFIX}/include/chomp ]; then
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
  ./install.sh ${PREFIX} || exit 1
  cd .. || exit 1
  echo CHomP installed.
else
  echo CHomP already installed.
fi

# cluster-delegator
echo
echo ==\> cluster-delegator
echo Checking for cluster-delegator.
if [ ! -d ${PREFIX}/include/delegator ] || [ ! -f ${PREFIX}/include/boost/ser\
ialization/unordered_set.hpp ]; then
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
  ./install.sh ${PREFIX} || exit 1
  cd ..
  echo cluster-delegator installed.
else
  echo cluster-delegator already installed.
fi

cd conley-morse-database

if [ ${PREFIX} != "/usr/local" ]; then
    echo "PREREQ:=${PREFIX}" > makefile.dep
else
    echo "PREREQ:=" > makefile.dep
fi
