#!/bin/bash
set -e
PWDD=$(pwd)
DIR=install-libcxx-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR




svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
svn co http://llvm.org/svn/llvm-project/libcxxabi/trunk libcxxabi

# build libc++abi
( cd libcxxabi && mkdir build && cd build && export CC=clang-5.0 CXX=clang++-5.0 && \
	cmake -DLIBCXXABI_LIBCXX_INCLUDES=../../libcxx/include \
	.. && make -j8 && make install --prefix $PREFIX )

# build libc++
( mkdir build && cd build && export CC=clang-5.0 CXX=clang++-5.0 && \
	cmake -DLLVM_PATH=../llvm \
	-DLIBCXX_CXX_ABI=libcxxabi \
	-DLIBCXX_CXX_ABI_INCLUDE_PATHS=../libcxxabi/include \
	-DLIBCXX_CXX_ABI_LIBRARY_PATH=../libcxxabi/build/lib \
	../libcxx && make -j8 && make install --prefix $PREFIX )




cd $PWDD && rm -rf $DIR
