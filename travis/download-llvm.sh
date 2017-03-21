#!/bin/bash
#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

isVersionGe ()
{
    [  $1 == `echo -e "$1\n$2" | sort -V -r | head -n1` ]
}

if isVersionGe $LLVM_VERSION 3.5; then
	LLVM_TAR=llvm-$LLVM_VERSION.src.tar.xz
else
	LLVM_TAR=llvm-$LLVM_VERSION.src.tar.gz
fi

if isVersionGe $LLVM_VERSION 3.9; then
    export LLVM_CMAKE_SUBDIR=lib/cmake/llvm
else
    export LLVM_CMAKE_SUBDIR=share/llvm/cmake
fi

LLVM_URL=http://releases.llvm.org/$LLVM_VERSION/$LLVM_TAR

wget --quiet $LLVM_URL
mkdir -p llvm
tar --strip-components=1 -xf $LLVM_TAR -C llvm

if [ "$TARGET_CPU" == "x86" ]; then
	export LLVM_BUILD_32_BITS="-DLLVM_BUILD_32_BITS=TRUE"
fi
