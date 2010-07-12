#!/bin/bash
##  This build tool was written by Hans J. Johnson hans-johnson@uiowa.edu
PROJECTNAME=DTIPrep
BUILDNAME=$(uname)_$(uname -m)-$(hostname -s)
SOURCE_DIR=$(dirname $0)
if [ "${SOURCE_DIR}" == "." ]; then
  SOURCE_DIR=$(pwd)
fi

if [ $# -lt 1 ]; then
  ABI="FAST"
else
  ABI=$1
fi

export CC=/usr/bin/gcc-4.2
export CXX=/usr/bin/g++-4.2
if [ ! -f ${CC} ] || [ ! -f ${CXX} ]; then
export CC=`which gcc`
export CXX=`which g++`
fi

## Removed -Wshadow from build to prevent qt warnings form hiding current errors
case ${ABI} in
  "PROFILE")
    CFLAGS="-Wall -Wstrict-prototypes -fprofile-arcs -ftest-coverage -pg  -UNDEBUG ${FLAGS_FROM_QT_BUILD}"
    CXXFLAGS="-Wall  -fprofile-arcs -ftest-coverage -pg -UNDEBUG ${FLAGS_FROM_QT_BUILD}"
    ;;
  "OPTDEBUG")
    CFLAGS="-Wstrict-prototypes -g -O1 ${FLAGS_FROM_QT_BUILD}"
    CXXFLAGS=" -g -O1 ${FLAGS_FROM_QT_BUILD}"
    ;;
  "DEBUG")
    CFLAGS=" -g ${FLAGS_FROM_QT_BUILD}"
    CXXFLAGS=" -g ${FLAGS_FROM_QT_BUILD}"
    ;;
  "FAST")
    CFLAGS="-DNDEBUG -O3 -msse -mmmx -msse2 -msse3  ${FLAGS_FROM_QT_BUILD}"
    CXXFLAGS="-DNDEBUG -O3 -msse -mmmx -msse2 -msse3  ${FLAGS_FROM_QT_BUILD}"
    ;;
  *)
    echo "INVALID ABI GIVEN"
    exit -1;
esac
## Auto-determine the number of processors on this computer
case "$(uname)" in
  "Linux")
  maxproc=$(grep processor /proc/cpuinfo | wc | awk '{print $1}')
  ;;
  "Darwin")
  maxproc=$(sysctl -n hw.ncpu)
  ;;
  *)
  echo "Platform not recognized"
  maxproc=1;
esac
if [ "x${maxproc}" == "x" ] || [ ${maxproc} -lt 1 ]; then
  maxproc=1;
fi

COMPILE_DIR=$(dirname ${SOURCE_DIR})/${PROJECTNAME}-COMPILE/
echo COMPILE_DIR $COMPILE_DIR SOURCE_DIR $SOURCE_DIR

FULL_SOURCE_PATH=$(readlink -f ${SOURCE_DIR})
FULL_SOURCE_PATH=${SOURCE_DIR}
echo FULL_SOURCE_PATH $FULL_SOURCE_PATH

ABI_DIR=${COMPILE_DIR}/$(uname)_$(uname -m)-${ABI}
mkdir -p ${ABI_DIR}

export QTDIR=/opt/qt-4.6.1
export QTDIR=/opt/Qt
export PATH=${QTDIR}/bin:${PATH}
#export QTDIR=/opt/QtCarbon
#export PATH=${QTDIR}/bin:${PATH}
#################################################################################
#Build ${PROJECTNAME}
pushd ${ABI_DIR}
##NOTE:  Using cmake and all comand line options.  Normally ccmake would be used.
CMAKE_MODULE_PATH=${LOCAL_PATH}/CMake CC=${CC} CXX=${CXX} CFLAGS=${CFLAGS} CXXFLAGS=${CXXFLAGS} \
cmake \
-DBUILD_SHARED_LIBS:BOOL=OFF \
-DBUILD_TESTING:BOOL=OFF \
-DQT_QMAKE_EXECUTABLE:FILEPATH=${QTDIR}/bin/qmake \
-DQT_SEARCH_PATH:FILEPATH=${QTDIR} \
-DDESIRED_QT_VERSION:STRING=4 \
${FULL_SOURCE_PATH}

case "$(uname)" in
  "Linux")
  maxproc=$(grep processor /proc/cpuinfo | wc | awk '{print $1}')
  ;;
  "Darwin")
  maxproc=$(sysctl -n hw.ncpu)
  ;;
  *)
  echo "Platform not recognized"
  maxproc=1;
esac
make -j ${maxproc}


