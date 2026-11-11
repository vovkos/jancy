::..............................................................................
::
::  This file is part of the AXL library.
::
::  AXL is distributed under the MIT license.
::  For details see accompanying license.txt file,
::  the public copy of which is also available at:
::  http://tibbo.com/downloads/archive/axl/license.txt
::
::..............................................................................

@echo off

set DOWNLOAD_DIR=c:\downloads
set DOWNLOAD_DIR_CMAKE=%DOWNLOAD_DIR:\=/%

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Dependency versions

set LUA_VERSION=5.4.7
set LUA_DOWNLOAD_URL=https://github.com/walterschell/Lua/archive/refs/tags/v%LUA_VERSION%.zip

set OPENSSL_VERSION=1.1.1.2100

set RAGEL_DOWNLOAD_URL=https://github.com/eloraiby/ragel-windows/raw/master/ragel.exe

set COMPARE_VERSIONS_PL=%~dp0compare-versions.pl

set LLVM_TOOLCHAIN=msvc17
perl %COMPARE_VERSIONS_PL% %LLVM_VERSION% 13
if %errorlevel% == -1 set LLVM_TOOLCHAIN=msvc15

set LLVM_DEBUG_SUFFIX=
if "%CONFIGURATION%" == "Debug" set LLVM_DEBUG_SUFFIX=-dbg

set LLVM_RELEASE_NAME=%LLVM_VERSION%-windows-%TARGET_CPU%-%LLVM_TOOLCHAIN%-msvcrt%LLVM_DEBUG_SUFFIX%
set LLVM_DOWNLOAD_FILE=%LLVM_RELEASE_NAME%.7z
set LLVM_DOWNLOAD_URL=https://github.com/vovkos/llvm-package-windows/releases/download/%LLVM_VERSION%/%LLVM_DOWNLOAD_FILE%

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: wget

echo Installing wget...

choco install wget --no-progress || exit

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Ragel

echo Downloading %RAGEL_DOWNLOAD_URL%...

mkdir %DOWNLOAD_DIR%
wget -nv %RAGEL_DOWNLOAD_URL% -O %DOWNLOAD_DIR%\ragel.exe || exit

echo set(RAGEL_EXE %DOWNLOAD_DIR_CMAKE%/ragel.exe) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Lua (CMake-based)

echo Downloading %LUA_DOWNLOAD_URL%...

mkdir %DOWNLOAD_DIR%\lua
wget -nv %LUA_DOWNLOAD_URL% -O %DOWNLOAD_DIR%\lua\lua.zip || exit
7z x -y %DOWNLOAD_DIR%\lua\lua.zip || exit
ren Lua-%LUA_VERSION% lua || exit

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: OpenSSL

if /i "%TARGET_CPU%" == "arm64" goto :openssl_arm64
if /i "%TARGET_CPU%" == "x86" goto :openssl_x86

set OPENSSL_DIR_SUFFIX=
set OPENSSL_DLL_SUFFIX=-1_1-x64
goto :openssl_install

:openssl_x86
set OPENSSL_DIR_SUFFIX=-Win32
set OPENSSL_DLL_SUFFIX=-1_1
goto :openssl_install

:: choco doesn't ship arm64 builds; on arm64 we use preinstalled OpenSSL 3
:: (libs are at lib/VC/<arch>/<crt>)

:openssl_arm64
set OPENSSL_DLL_SUFFIX=-3-arm64
set OPENSSL_ROOT_DIR=C:/Program Files/OpenSSL

if /i "%CONFIGURATION%" == "Debug" (
	set OPENSSL_LIB_SUBDIR=lib/VC/arm64/MDd
) else (
	set OPENSSL_LIB_SUBDIR=lib/VC/arm64/MD
)

goto :openssl_finalize

:openssl_install

echo Installing OpenSSL %OPENSSL_VERSION%...

choco install openssl --version=%OPENSSL_VERSION% --no-progress -- %CHOCO_PLATFORM% || exit

set OPENSSL_ROOT_DIR=C:/Program Files%PROGRAM_FILES_DIR_SUFFIX%/OpenSSL%OPENSSL_DIR_SUFFIX%
set OPENSSL_LIB_SUBDIR=lib

:openssl_finalize

echo set(OPENSSL_INC_DIR "%OPENSSL_ROOT_DIR%/include") >> paths.cmake
echo set(OPENSSL_LIB_DIR "%OPENSSL_ROOT_DIR%/%OPENSSL_LIB_SUBDIR%") >> paths.cmake
echo set(OPENSSL_DLL_DIR "%OPENSSL_ROOT_DIR%/bin") >> paths.cmake
echo set(OPENSSL_CRYPTO_LIB_NAME libcrypto) >> paths.cmake
echo set(OPENSSL_CRYPTO_DLL_NAME libcrypto%OPENSSL_DLL_SUFFIX%) >> paths.cmake
echo set(OPENSSL_SSL_LIB_NAME libssl) >> paths.cmake
echo set(OPENSSL_SSL_DLL_NAME libssl%OPENSSL_DLL_SUFFIX%) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: LLVM

echo Downloading %LLVM_DOWNLOAD_URL%...

wget -nv %LLVM_DOWNLOAD_URL% -O %DOWNLOAD_DIR%\%LLVM_DOWNLOAD_FILE% || exit
7z x -y %DOWNLOAD_DIR%\%LLVM_DOWNLOAD_FILE% -o%DOWNLOAD_DIR% || exit
ren %DOWNLOAD_DIR%\%LLVM_RELEASE_NAME% llvm || exit

perl %COMPARE_VERSIONS_PL% %LLVM_VERSION% 3.5
if %errorlevel% == -1 goto llvm34x

echo set(LLVM_CMAKE_DIR %DOWNLOAD_DIR_CMAKE%/llvm/lib/cmake/llvm) >> paths.cmake
goto :eof

:llvm34x

echo set(LLVM_INC_DIR %DOWNLOAD_DIR_CMAKE%/llvm/include) >> paths.cmake
echo set(LLVM_LIB_DIR %DOWNLOAD_DIR_CMAKE%/llvm/lib) >> paths.cmake
echo set(LLVM_CMAKE_DIR %DOWNLOAD_DIR_CMAKE%/llvm/share/llvm/cmake) >> paths.cmake
echo set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_CMAKE_DIR}) >> paths.cmake

::..............................................................................
