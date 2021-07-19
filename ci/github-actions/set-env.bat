:: .............................................................................
::
::  This file is part of the AXL library.
::
::  AXL is distributed under the MIT license.
::  For details see accompanying license.txt file,
::  the public copy of which is also available at:
::  http://tibbo.com/downloads/archive/axl/license.txt
::
:: .............................................................................

@echo off

:loop

if "%1" == "" goto :finalize
if /i "%1" == "msvc10" goto :msvc10
if /i "%1" == "msvc12" goto :msvc12
if /i "%1" == "msvc14" goto :msvc14
if /i "%1" == "msvc15" goto :msvc15
if /i "%1" == "x86" goto :x86
if /i "%1" == "i386" goto :x86
if /i "%1" == "amd64" goto :amd64
if /i "%1" == "x86_64" goto :amd64
if /i "%1" == "x64" goto :amd64

echo Invalid argument: '%1'
exit -1

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Toolchain

:msvc10
set TOOLCHAIN=msvc10
set CMAKE_GENERATOR=Visual Studio 10 2010
set LUA_TOOLCHAIN=dll10
shift
goto :loop

:msvc12
set TOOLCHAIN=msvc12
set CMAKE_GENERATOR=Visual Studio 12 2013
set LUA_TOOLCHAIN=dll12
shift
goto :loop

:msvc14
set TOOLCHAIN=msvc14
set CMAKE_GENERATOR=Visual Studio 14 2015
set LUA_TOOLCHAIN=dll14
shift
goto :loop

:msvc15
set TOOLCHAIN=msvc15
set CMAKE_GENERATOR=Visual Studio 15 2017
set LUA_TOOLCHAIN=dll15
shift
goto :loop

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Platform

:x86
set TARGET_CPU=x86
set CMAKE_GENERATOR_SUFFIX=
set LUA_PLATFORM=Win32
set OPENSSL_PLATFORM=Win32
set OPENSSL_DLL_SUFFIX=-1_1
set CHOCO_PLATFORM=--x86
set PROGRAM_FILES_DIR_SUFFIX= (x86)
shift
goto :loop

:amd64
set TARGET_CPU=amd64
set CMAKE_GENERATOR_SUFFIX= Win64
set LUA_PLATFORM=Win64
set OPENSSL_PLATFORM=Win64
set OPENSSL_DLL_SUFFIX=-1_1-x64
set CHOCO_PLATFORM=
set PROGRAM_FILES_DIR_SUFFIX=
shift
goto :loop

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:finalize

if "%TOOLCHAIN%" == "" goto :msvc14
if "%TARGET_CPU%" == "" goto :amd64
if "%CONFIGURATION%" == "" (set CONFIGURATION=Release)

set LLVM_DEBUG_SUFFIX=
if "%CONFIGURATION%" == "Debug" set LLVM_DEBUG_SUFFIX=-dbg
set LLVM_RELEASE_NAME=llvm-%LLVM_VERSION%-windows-%TARGET_CPU%-%TOOLCHAIN%-msvcrt%LLVM_DEBUG_SUFFIX%
set LLVM_DOWNLOAD_FILE=%LLVM_RELEASE_NAME%.7z
set LLVM_DOWNLOAD_URL=https://github.com/vovkos/llvm-package-windows/releases/download/llvm-%LLVM_VERSION%/%LLVM_DOWNLOAD_FILE%

set LUA_VERSION=5.3.5
set LUA_LIB_NAME=lua53
set LUA_DOWNLOAD_URL=https://sourceforge.net/projects/luabinaries/files/%LUA_VERSION%/Windows%%20Libraries/Dynamic/lua-%LUA_VERSION%_%LUA_PLATFORM%_%LUA_TOOLCHAIN%_lib.zip/download

set RAGEL_DOWNLOAD_URL=https://github.com/eloraiby/ragel-windows/raw/master/ragel.exe

set CMAKE_CONFIGURE_FLAGS=-G "%CMAKE_GENERATOR%%CMAKE_GENERATOR_SUFFIX%"

set CMAKE_BUILD_FLAGS= ^
	--config %CONFIGURATION% ^
	-- ^
	/nologo ^
	/verbosity:minimal ^
	/consoleloggerparameters:Summary

echo ---------------------------------------------------------------------------
echo LLVM_VERSION:       %LLVM_VERSION%
echo LLVM_DOWNLOAD_URL:  %LLVM_DOWNLOAD_URL%
echo LUA_LIB_NAME:         %LUA_LIB_NAME%
echo LUA_DOWNLOAD_URL:     %LUA_DOWNLOAD_URL%
echo RAGEL_DOWNLOAD_URL:   %RAGEL_DOWNLOAD_URL%
echo ---------------------------------------------------------------------------
