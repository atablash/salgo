#!/bin/bash
PWDD=$(pwd)
DIR=install-glog-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR




git clone https://github.com/google/glog.git --depth 1

( cd glog && ./autogen.sh && ./configure && make -j8 && make install DESTDIR=$PREFIX )




cd $PWDD && rm -rf $DIR


