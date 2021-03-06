#!/bin/bash

SOURCE_DIR=$SOURCE_DIR_BASE/VTK-6.1.0

if [ $BUILD_TYPE = debug ]
then
    CMAKE_BUILD_TYPE=Debug
    BUILD_DIR=$SOURCE_DIR-build-debug
fi
if [ $BUILD_TYPE = release ]
then
    CMAKE_BUILD_TYPE=RelWithDebInfo
    BUILD_DIR=$SOURCE_DIR-build-release
fi

################ Nothing should need to be changed below this line ################

CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE \
               -DCMAKE_INSTALL_PREFIX:PATH=$SDK_INSTALL_PREFIX \
               -DCMAKE_PREFIX_PATH:PATH=$QTDIR \
               -DCMAKE_CXX_FLAGS=-DGLX_GLXEXT_LEGACY \
               -DVTK_Group_Qt:BOOL=TRUE \
               -DVTK_QT_VERSION:STRING=5"

VTKCMAKEDIR=$SDK_INSTALL_PREFIX/lib/cmake/vtk-6.1
VTKLIBDIR=$SDK_INSTALL_PREFIX/lib
VTKINCLUDEDIR=$SDK_INSTALL_PREFIX/include/vtk-6.1
