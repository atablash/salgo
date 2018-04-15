#!/bin/bash
set -e
PWDD=$(pwd)
DIR=install-cmake-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR



wget https://cmake.org/files/v3.11/cmake-3.11.0.tar.gz
tar xvf cmake*.tar.gz >/dev/null
( cd cmake-* && ./bootstrap --prefix=$PREFIX/usr/local && make -j8 && make install )




cd $PWDD && rm -rf $DIR
