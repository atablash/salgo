#!/bin/bash
set -e
PWDD=$(pwd)
DIR=install-libcxx-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR




svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm           >/dev/null
svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx       >/dev/null
svn co http://llvm.org/svn/llvm-project/libcxxabi/trunk libcxxabi >/dev/null

# build libc++abi
( cd libcxxabi && mkdir build && cd build &&  \
	cmake -DLIBCXXABI_LIBCXX_INCLUDES=../../libcxx/include  \
	.. && make -j8 && make install DESTDIR=$PREFIX ) \
	2>/dev/null

# build libc++
( mkdir build && cd build &&  \
	cmake -DLLVM_PATH=../llvm  \
	-DLIBCXX_CXX_ABI=libcxxabi  \
	-DLIBCXX_CXX_ABI_INCLUDE_PATHS=../libcxxabi/include  \
	-DLIBCXX_CXX_ABI_LIBRARY_PATH=../libcxxabi/build/lib  \
	../libcxx && make -j8 && make install DESTDIR=$PREFIX ) \
	2>/dev/null




cd $PWDD && rm -rf $DIR
