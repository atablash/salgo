#!/bin/bash
set -e
PWDD=$(pwd)
DIR=install-eigen-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR




# download Eigen and copy header files
wget http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2
tar xvf *.tar.bz2

mkdir -p $PREFIX/usr/local/include/
mv eigen-*/Eigen $PREFIX/usr/local/include/





cd $PWDD && rm -rf $DIR


