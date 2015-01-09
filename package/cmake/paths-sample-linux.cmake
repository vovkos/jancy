# this file is provided for reference only.
# fill variables with paths on your build machine.
# if library/tool is not needed, leave the path empty.

#..............................................................................

set (LLVM_VERSION      3.3)
set (QT_VERSION        5.0.2)
set (AXL_VERSION       5.3.2)
set (BULLDOZER_VERSION 1.1.1)

#..............................................................................

if ("${TARGET_CPU}" STREQUAL "amd64")
	set (LLVM_CMAKE_DIR   "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/prj/make/share/llvm/cmake")
	set (LLVM_INC_DIR     "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/include")
	set (LLVM_INC_DIR_2   "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/prj/make/include")
	set (LLVM_LIB_DIR     "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/prj/make/lib")
	set (QT_CMAKE_DIR     "/home/vladimir/Develop/qt/qt-${QT_VERSION}/${QT_VERSION}/gcc_64/lib/cmake")
else ()
	set (LLVM_CMAKE_DIR   "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/prj/make.x86.rel/share/llvm/cm
	set (LLVM_INC_DIR     "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/include")
	set (LLVM_INC_DIR_2   "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/prj/make.x86.rel/include")
	set (LLVM_LIB_DIR     "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/prj/make.x86.rel/lib")
	set (QT_CMAKE_DIR     "/home/vladimir/Develop/qt/qt-${QT_VERSION}.x86/${QT_VERSION}/gcc/lib/cmake")
endif ()

set (AXL_CMAKE_DIR        "${CMAKE_CURRENT_LIST_DIR}/../../../../axl/axl-${AXL_VERSION}/cmake/latest")
set (BULLDOZER_CMAKE_DIR  "${CMAKE_CURRENT_LIST_DIR}/../../../../bulldozer/bulldozer-${BULLDOZER_VERSION}/cmake")

set (PERL_EXE             "perl")
set (RAGEL_EXE            "ragel")

#..............................................................................