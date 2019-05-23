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
	DIST_SUFFIX=
	CPU_SUFFIX=
	CC_SUFFIX=
else
	DIST_SUFFIX=-$TRAVIS_DIST
	CPU_SUFFIX=-$TARGET_CPU
	CC_SUFFIX=-$CC
fi

if [ $BUILD_CONFIGURATION != "Debug" ]; then
	DEBUG_SUFFIX=
else
	DEBUG_SUFFIX=-dbg
fi

LLVM_RELEASE=llvm-$LLVM_VERSION-$TRAVIS_OS_NAME$DIST_SUFFIX
LLVM_TAR=$LLVM_RELEASE$CPU_SUFFIX$CC_SUFFIX$DEBUG_SUFFIX.tar.xz
LLVM_URL=https://github.com/vovkos/llvm-package-travis/releases/download/$LLVM_RELEASE/$LLVM_TAR

echo getting LLVM from: $LLVM_URL

wget --quiet $LLVM_URL
mkdir -p llvm
tar --strip-components=1 -xf $LLVM_TAR -C llvm

if [[ $LLVM_VERSION < "3.5.0" ]]; then
	echo "set (LLVM_INC_DIR $(pwd)/llvm/include)" >> paths.cmake
	echo "set (LLVM_LIB_DIR $(pwd)/llvm/lib)" >> paths.cmake
	echo "set (LLVM_CMAKE_DIR $(pwd)/llvm/share/llvm/cmake)" >> paths.cmake
	echo "set (CMAKE_MODULE_PATH \${CMAKE_MODULE_PATH} \${LLVM_CMAKE_DIR})" >> paths.cmake
else
	echo "set (LLVM_CMAKE_DIR $(pwd)/llvm/lib/cmake/llvm)" >> paths.cmake
fi
