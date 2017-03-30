::..............................................................................
::
::  This file is part of the Jancy toolkit.
::
::  Jancy is distributed under the MIT license.
::  For details see accompanying license.txt file,
::  the public copy of which is also available at:
::  http://tibbo.com/downloads/archive/jancy/license.txt
::
::..............................................................................

@echo off

set THIS_DIR=%CD%

set LLVM_CMAKE_FLAGS= ^
	-G "%CMAKE_GENERATOR%%CMAKE_GENERATOR_SUFFIX%" ^
	-DCMAKE_BUILD_TYPE=%BUILD_CONFIGURATION% ^
	-DLLVM_BUILD_32_BITS=%LLVM_BUILD_32_BITS% ^
	-DLLVM_USE_CRT_DEBUG=MTd ^
	-DLLVM_USE_CRT_RELEASE=MT ^
	-DLLVM_USE_CRT_MINSIZEREL=MT ^
	-DLLVM_USE_CRT_RELWITHDEBINFO=MT ^
	-DLLVM_TARGETS_TO_BUILD=X86 ^
	-DLLVM_ENABLE_TERMINFO=OFF ^
	-DLLVM_ENABLE_ZLIB=OFF ^
	-DLLVM_INCLUDE_DOCS=OFF ^
	-DLLVM_INCLUDE_EXAMPLES=OFF ^
	-DLLVM_INCLUDE_TESTS=OFF ^
	-DLLVM_INCLUDE_TOOLS=OFF ^
	-DLLVM_INCLUDE_UTILS=OFF

cd %DOWNLOAD_DIR%\llvm
mkdir build
cd build
cmake .. %LLVM_CMAKE_FLAGS%
cmake --build . -- %MSBUILD_FLAGS%

cd %THIS_DIR%
echo set (LLVM_CMAKE_DIR %DOWNLOAD_DIR_CMAKE%/llvm/build/%LLVM_CMAKE_SUBDIR%) >> paths.cmake
echo set (LLVM_INC_DIR %DOWNLOAD_DIR_CMAKE%/llvm/include %DOWNLOAD_DIR_CMAKE%/llvm/build/include) >> paths.cmake
echo set (LLVM_LIB_DIR %DOWNLOAD_DIR_CMAKE%/llvm/build/lib/%CONFIGURATION%) >> paths.cmake
