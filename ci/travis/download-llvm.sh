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

if [ $TRAVIS_OS_NAME == "osx" ]; then
	GNUSORT=gsort
	CPU_SUFFIX=
	CC_SUFFIX=
else
	GNUSORT=sort
	CPU_SUFFIX=-$TARGET_CPU
	CC_SUFFIX=-$CC
fi

isVersionGe ()
{
    [ $1 == `echo -e "$1\n$2" | $GNUSORT -V -r | head -n1` ]
}

if [ $BUILD_CONFIGURATION != "Debug" ]; then
	DEBUG_SUFFIX=
	TAR_FILE_EXT=.xz
else
	DEBUG_SUFFIX=-dbg

	if isVersionGe $LLVM_VERSION 3.8; then
		TAR_FILE_EXT=.gz
	else
		TAR_FILE_EXT=.xz
	fi
fi

LLVM_TAR=llvm-$LLVM_VERSION-$TRAVIS_OS_NAME$CPU_SUFFIX$CC_SUFFIX$DEBUG_SUFFIX.tar$TAR_FILE_EXT
LLVM_URL=https://github.com/vovkos/llvm-package-travis/releases/download/llvm-$LLVM_VERSION/$LLVM_TAR

if isVersionGe $LLVM_VERSION 3.9; then
    LLVM_CMAKE_SUBDIR=lib/cmake/llvm
else
    LLVM_CMAKE_SUBDIR=share/llvm/cmake
fi

echo getting LLVM from: $LLVM_URL

wget --quiet $LLVM_URL
mkdir -p llvm
tar --strip-components=1 -xf $LLVM_TAR -C llvm

echo "set (LLVM_CMAKE_DIR $(pwd)/llvm/$LLVM_CMAKE_SUBDIR)" >> paths.cmake
