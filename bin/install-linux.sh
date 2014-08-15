# Linux Installer script for conley-morse-database
PREFIX=$1
cd ..
# Install Boost
if [ ! -d ${PREFIX}/include/boost ]; then
  wget http://downloads.sourceforge.net/project/boost/boost/1.56.0/boost_1_56_0.tar.gz
  tar xvfz boost_1_56_0.tar.gz
  cd boost_1_56_0
  ./bootstrap.sh --prefix=${PREFIX}
  ./b2 install
  cd ..
fi

# Install openmpi
if [ "`which mpicxx`" == "" ]; then
  wget http://www.open-mpi.org/software/ompi/v1.8/downloads/openmpi-1.8.1.tar.gz
  tar xvfz openmpi-1.8.1
  cd openmpi-1.8.1
  ./configure --prefix=${PREFIX}
  make all install
  cd ..
fi

# Install cmake
if [ "`which cmake`" == "" ]; then
  INSTALLCMAKE=yes
  echo Will make cmake
else
  CMAKEVERSION=`cmake --version | grep -o "[0-9]\.[0-9]"`
  if [ ` echo "$CMAKEVERSION < 3.0" | bc ` == "1" ]; then
      echo Will make cmake
      INSTALLCMAKE=yes
  fi
fi

if [ $INSTALLCMAKE == "yes" ]; then
  wget http://www.cmake.org/files/v3.0/cmake-3.0.1.tar.gz
  tar xvfz cmake-3.0.1.tar.gz
  cd cmake-3.0.1
  ./bootstrap --prefix=${PREFIX}
  make
  make install
  cd ..
fi

PATH=${PREFIX}/bin:${PATH}
export PATH

# Install "sdsl"
if [ ! -d ${PREFIX}/include/sdsl ]; then
  git clone https://github.com/simongog/sdsl-lite.git
  cd sdsl-lite
  ./install.sh ${PREFIX}
  cd ..
fi

# Install "CHomP"
if [ ! -d ${PREFIX}/include/chomp ]; then
  git clone https://github.com/sharker81/CHomP.git
  cd CHomP
  ./install.sh ${PREFIX}
  cd ..
fi

# Install "cluster-delegator"
if [ ! -d ${PREFIX}/include/delegator ]; then
  git clone https://github.com/sharker81/cluster-delegator.git
  cd cluster-delegator
  ./install.sh ${PREFIX}
  cd ..
fi

cd conley-morse-database

if [ ${PREFIX} != "/usr/local" ]; then
    echo "PREREQ:=${PREFIX}" > makefile.dep
else
    echo "PREREQ:=" > makefile.dep
fi
