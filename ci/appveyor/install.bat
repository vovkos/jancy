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

set DOWNLOAD_DIR=c:\downloads
set DOWNLOAD_DIR_CMAKE=%DOWNLOAD_DIR:\=/%

if not exist %DOWNLOAD_DIR% mkdir %DOWNLOAD_DIR%

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Get rid of annoying Xamarin build warnings

if exist "c:\Program Files (x86)\MSBuild\14.0\Microsoft.Common.targets\ImportAfter\Xamarin.Common.targets" (
	del "c:\Program Files (x86)\MSBuild\14.0\Microsoft.Common.targets\ImportAfter\Xamarin.Common.targets"
)

if exist "c:\Program Files (x86)\MSBuild\4.0\Microsoft.Common.targets\ImportAfter\Xamarin.Common.targets" (
	del "c:\Program Files (x86)\MSBuild\4.0\Microsoft.Common.targets\ImportAfter\Xamarin.Common.targets"
)

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Ragel

appveyor DownloadFile %RAGEL_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\ragel.exe
echo set (RAGEL_EXE %DOWNLOAD_DIR_CMAKE%/ragel.exe) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: NASM

appveyor DownloadFile %NASM_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\%NASM_DOWNLOAD_FILE%
7z x -y %DOWNLOAD_DIR%\%NASM_DOWNLOAD_FILE% -o%DOWNLOAD_DIR%

dir %DOWNLOAD_DIR%
dir %DOWNLOAD_DIR_CMAKE%\nasm-%NASM_VERSION%

echo set (NASM_EXE %DOWNLOAD_DIR_CMAKE%/nasm-%NASM_VERSION%/nasm.exe) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Lua

mkdir %DOWNLOAD_DIR%\lua
appveyor DownloadFile %LUA_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\lua\%LUA_DOWNLOAD_FILE%
7z x -y %DOWNLOAD_DIR%\lua\%LUA_DOWNLOAD_FILE% -o%DOWNLOAD_DIR%\lua

echo set (LUA_INC_DIR %DOWNLOAD_DIR_CMAKE%/lua/include) >> paths.cmake
echo set (LUA_LIB_DIR %DOWNLOAD_DIR_CMAKE%/lua) >> paths.cmake
echo set (LUA_DLL_DIR %DOWNLOAD_DIR_CMAKE%/lua) >> paths.cmake
echo set (LUA_LIB_NAME %LUA_LIB_NAME%) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: OpenSSL

set OPENSSL_DIR_CMAKE=%OPENSSL_DIR:\=/%

echo set (OPENSSL_INC_DIR %OPENSSL_DIR_CMAKE%/include) >> paths.cmake
echo set (OPENSSL_LIB_DIR %OPENSSL_DIR_CMAKE%/lib) >> paths.cmake
echo set (OPENSSL_DLL_DIR %OPENSSL_DIR_CMAKE%/bin) >> paths.cmake
echo set (OPENSSL_CRYPTO_LIB_NAME libeay32) >> paths.cmake
echo set (OPENSSL_CRYPTO_DLL_NAME libeay32) >> paths.cmake
echo set (OPENSSL_SSL_LIB_NAME ssleay32) >> paths.cmake
echo set (OPENSSL_SSL_DLL_NAME ssleay32) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: LLVM

appveyor DownloadFile %LLVM_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\%LLVM_DOWNLOAD_FILE%
7z x -y %DOWNLOAD_DIR%\%LLVM_DOWNLOAD_FILE% -o%DOWNLOAD_DIR%
ren %DOWNLOAD_DIR%\%LLVM_RELEASE_NAME% llvm

if "%LLVM_VERSION%" lss "3.5.0" goto llvm34x

echo set (LLVM_CMAKE_DIR %DOWNLOAD_DIR_CMAKE%/llvm/lib/cmake/llvm) >> paths.cmake
goto :eof

:llvm34x

echo set (LLVM_INC_DIR %DOWNLOAD_DIR_CMAKE%/llvm/include) >> paths.cmake
echo set (LLVM_LIB_DIR %DOWNLOAD_DIR_CMAKE%/llvm/lib) >> paths.cmake
echo set (LLVM_CMAKE_DIR %DOWNLOAD_DIR_CMAKE%/llvm/share/llvm/cmake) >> paths.cmake
echo set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_CMAKE_DIR}) >> paths.cmake
