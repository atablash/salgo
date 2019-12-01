#!/bin/bash
set -e
PWDD=$(pwd)
DIR=install-libcxx-temp
PREFIX=$PWDD/INSTALL_ROOT
rm -rf $DIR && mkdir $DIR && cd $DIR



git clone https://github.com/llvm/llvm-project.git --branch llvmorg-8.0.1 --depth 1

mv llvm-project/llvm .
mv llvm-project/libcxx .
mv llvm-project/libcxxabi .

# build libc++abi
( cd libcxxabi && mkdir build && cd build &&  \
	cmake -DLIBCXXABI_LIBCXX_INCLUDES=../../libcxx/include  \
	.. && make -j8 && make install DESTDIR=$PREFIX ) #\
	#2>/dev/null

# build libc++
( mkdir build && cd build &&  \
	cmake -DLLVM_PATH=../llvm  \
	-DLIBCXX_CXX_ABI=libcxxabi  \
	-DLIBCXX_CXX_ABI_INCLUDE_PATHS=../libcxxabi/include  \
	-DLIBCXX_CXX_ABI_LIBRARY_PATH=../libcxxabi/build/lib  \
	../libcxx && make -j8 && make install DESTDIR=$PREFIX ) #\
	#2>/dev/null




cd $PWDD && rm -rf $DIR
