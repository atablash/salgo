#!/bin/bash
PWDD=`pwd`
DIR=install-google-benchmark-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR




git clone https://github.com/google/benchmark.git google-benchmark --depth 1

( cd google-benchmark && \
	cmake . \
	-DBENCHMARK_ENABLE_GTEST_TESTS=OFF \
	-DCMAKE_BUILD_TYPE=RELEASE \
	$1 \
 	&& make -j8 && make install DESTDIR=$PREFIX )




cd $PWDD && rm -rf $DIR
