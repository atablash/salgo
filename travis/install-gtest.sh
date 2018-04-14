#!/bin/bash
set -e
PWDD=$(pwd)
DIR=install-gtest-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR


#
# build gtest (https://askubuntu.com/questions/145887/why-no-library-files-installed-for-google-test?newreg=b9a4644b541d4d99aac52be1822d9b2b)
# (actually just pull the new version from git)
#
git clone https://github.com/google/googletest.git --depth 1

echo "cmake . -DCMAKE_BUILD_TYPE=Release $1"


( cd googletest && \
	cmake . -DCMAKE_BUILD_TYPE=Release $1 && make -j8 && make install DESTDIR=$PREFIX )




cd $PWDD && rm -rf $DIR


