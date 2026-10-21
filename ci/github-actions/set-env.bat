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

:: save CI shell script dir before we shift args

set CI_SH_DIR=%~dp0

:loop

if "%1" == "" goto :finalize

if /i "%1" == "amd64" goto :amd64
if /i "%1" == "x86_64" goto :amd64
if /i "%1" == "x64" goto :amd64
if /i "%1" == "x86" goto :x86
if /i "%1" == "i386" goto :x86
if /i "%1" == "arm64" goto :arm64
if /i "%1" == "aarch64" goto :arm64
if /i "%1" == "msvc17" goto :msvc17
if /i "%1" == "msvc18" goto :msvc18
if /i "%1" == "v142" goto :toolset
if /i "%1" == "v143" goto :toolset
if /i "%1" == "v145" goto :toolset

echo Invalid argument: '%1'
exit -1

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: CPU

:x86
set TARGET_CPU=x86
set CMAKE_ARCH_OPTIONS=-A Win32
set OPENSSL_DIR_SUFFIX=-Win32
set OPENSSL_DLL_SUFFIX=-1_1
set CHOCO_PLATFORM=--x86
set PROGRAM_FILES_DIR_SUFFIX= (x86)
shift
goto :loop

:amd64
set TARGET_CPU=amd64
set CMAKE_ARCH_OPTIONS=-A x64
set OPENSSL_DIR_SUFFIX=
set OPENSSL_DLL_SUFFIX=-1_1-x64
set CHOCO_PLATFORM=
set PROGRAM_FILES_DIR_SUFFIX=
shift
goto :loop

:arm64
set TARGET_CPU=arm64
set CMAKE_ARCH_OPTIONS=-A ARM64
set OPENSSL_DIR_SUFFIX=
set OPENSSL_DLL_SUFFIX=-3-arm64
set CHOCO_PLATFORM=
set PROGRAM_FILES_DIR_SUFFIX=
shift
goto :loop

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Toolchain

:msvc17
set TOOLCHAIN=msvc17
set CMAKE_GENERATOR=Visual Studio 17 2022
shift
goto :loop

:msvc18
set TOOLCHAIN=msvc18
set CMAKE_GENERATOR=Visual Studio 18 2026
shift
goto :loop

:toolset
set CMAKE_TOOLSET_OPTIONS=-T %1
shift
goto :loop

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:finalize

if "%TARGET_CPU%" == "" goto :amd64
if "%TOOLCHAIN%" == "" goto :msvc17
if "%CONFIGURATION%" == "" (set CONFIGURATION=Release)
set COMPARE_VERSIONS_PL=%CI_SH_DIR%\compare-versions.pl

set LLVM_TOOLCHAIN=msvc17
perl %COMPARE_VERSIONS_PL% %LLVM_VERSION% 13
if %errorlevel% == -1 set LLVM_TOOLCHAIN=msvc15

set LLVM_DEBUG_SUFFIX=
if "%CONFIGURATION%" == "Debug" set LLVM_DEBUG_SUFFIX=-dbg

set LLVM_RELEASE_NAME=%LLVM_VERSION%-windows-%TARGET_CPU%-%LLVM_TOOLCHAIN%-msvcrt%LLVM_DEBUG_SUFFIX%
set LLVM_DOWNLOAD_FILE=%LLVM_RELEASE_NAME%.7z
set LLVM_DOWNLOAD_URL=https://github.com/vovkos/llvm-package-windows/releases/download/%LLVM_VERSION%/%LLVM_DOWNLOAD_FILE%

set LUA_VERSION=5.4.7
set LUA_DOWNLOAD_URL=https://github.com/walterschell/Lua/archive/refs/tags/v%LUA_VERSION%.zip

set OPENSSL_VERSION=1.1.1.2100

set RAGEL_DOWNLOAD_URL=https://github.com/eloraiby/ragel-windows/raw/master/ragel.exe

set CMAKE_CONFIGURE_FLAGS=-G "%CMAKE_GENERATOR%" %CMAKE_ARCH_OPTIONS% %CMAKE_TOOLSET_OPTIONS%

set CMAKE_BUILD_FLAGS= ^
	--config %CONFIGURATION% ^
	-- ^
	/nologo ^
	/verbosity:minimal ^
	/consoleloggerparameters:Summary

echo ---------------------------------------------------------------------------
echo LLVM_VERSION:       %LLVM_VERSION%
echo LLVM_DOWNLOAD_URL:  %LLVM_DOWNLOAD_URL%
echo LUA_DOWNLOAD_URL:   %LUA_DOWNLOAD_URL%
echo OPENSSL_VERSION:    %OPENSSL_VERSION%
echo RAGEL_DOWNLOAD_URL: %RAGEL_DOWNLOAD_URL%
echo ---------------------------------------------------------------------------
