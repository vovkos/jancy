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

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Ragel

mkdir %DOWNLOAD_DIR%\ragel
appveyor DownloadFile %RAGEL_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\ragel.exe

echo set (RAGEL_EXE %DOWNLOAD_DIR_CMAKE%/ragel.exe) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Lua

mkdir %DOWNLOAD_DIR%\lua
appveyor DownloadFile %LUA_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\lua\%LUA_DOWNLOAD_FILE%
7z x -y %DOWNLOAD_DIR%\lua\%LUA_DOWNLOAD_FILE% -o%DOWNLOAD_DIR%\lua

echo set (LUA_INC_DIR %DOWNLOAD_DIR_CMAKE%/lua/include) >> paths.cmake
echo set (LUA_LIB_DIR %DOWNLOAD_DIR_CMAKE%/lua) >> paths.cmake
echo set (LUA_LIB_NAME %LUA_LIB_NAME%) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: LLVM

appveyor DownloadFile %LLVM_DOWNLOAD_URL% -FileName %DOWNLOAD_DIR%\%LLVM_DOWNLOAD_FILE%
7z x -y %DOWNLOAD_DIR%\%LLVM_DOWNLOAD_FILE% -o%DOWNLOAD_DIR%
ren %DOWNLOAD_DIR%\%LLVM_RELEASE_NAME% llvm

echo set (LLVM_CMAKE_DIR %DOWNLOAD_DIR_CMAKE%/llvm/%LLVM_CMAKE_SUBDIR%) >> paths.cmake

:: . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

:: Get rid of Xamarin annoying build warnings

del "c:\Program Files (x86)\MSBuild\14.0\Microsoft.Common.targets\ImportAfter\Xamarin.Common.targets"
del "c:\Program Files (x86)\MSBuild\4.0\Microsoft.Common.targets\ImportAfter\Xamarin.Common.targets"
