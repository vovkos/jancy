# this file is provided for reference only.
# fill variables with paths on your build machine.
# if library/tool is not needed, leave the path empty.

#..............................................................................

set (LLVM_VERSION      3.3)
set (QT_VERSION        5.0.2)
set (AXL_VERSION       5.3.1)
set (BULLDOZER_VERSION 1.1.1)
set (RAGEL_VERSION     6.7)

#..............................................................................

if ("${TARGET_CPU}" STREQUAL "amd64")
	set (LLVM_INC_DIR     "c:/Develop/llvm/llvm-${LLVM_VERSION}/include")
	set (LLVM_INC_DIR_2   "c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10-amd64/include")
	set (LLVM_LIB_DIR     "c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10-amd64/lib/${CONFIGURATION_SUFFIX}")
	set (LLVM_CMAKE_DIR   "c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10-amd64/share/llvm/cmake")
	set (QT_CMAKE_DIR     "c:/Develop/qt/qt-${QT_VERSION}/build/amd64/qtbase/lib/cmake")
	set (QT_DLL_DIR       "c:/Develop/qt/qt-${QT_VERSION}/build/amd64/qtbase/lib")
else ()
	set (LLVM_INC_DIR     "c:/Develop/llvm/llvm-${LLVM_VERSION}/include")
	set (LLVM_INC_DIR_2   "c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10/include")
	set (LLVM_LIB_DIR     "c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10/lib/${CONFIGURATION_SUFFIX}")
	set (LLVM_CMAKE_DIR   "c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10/share/llvm/cmake")
	set (QT_CMAKE_DIR     "c:/Develop/qt/qt-${QT_VERSION}/build/x86/qtbase/lib/cmake")
	set (QT_DLL_DIR       "c:/Develop/qt/qt-${QT_VERSION}/build/x86/qtbase/lib")
endif()

set (AXL_CMAKE_DIR        "${CMAKE_CURRENT_LIST_DIR}/../../../../axl/axl-${AXL_VERSION}/cmake/latest")
set (BULLDOZER_CMAKE_DIR  "${CMAKE_CURRENT_LIST_DIR}/../../../../bulldozer/bulldozer-${BULLDOZER_VERSION}/cmake")

set (PERL_EXE             "c:/Develop/ActivePerl/bin/perl.exe")
set (RAGEL_EXE            "c:/Develop/ragel/ragel-${RAGEL_VERSION}/ragel.exe")

#..............................................................................
