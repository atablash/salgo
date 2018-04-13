#!/bin/bash
PWD=$(pwd)
DIR=install-gtest-temp
PREFIX=$PWD/INSTALL_ROOT
mkdir $DIR && cd $DIR


#
# build gtest (https://askubuntu.com/questions/145887/why-no-library-files-installed-for-google-test?newreg=b9a4644b541d4d99aac52be1822d9b2b)
# (actually just pull the new version from git)
#
git clone https://github.com/google/googletest.git --depth 1

( cd googletest && \
	cmake . \
	-DCMAKE_BUILD_TYPE=Release \
	$1 \
	&& make -j8 && make install DESTDIR=$PREFIX )




#cd $PWD && rm -rf $DIR


