# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

project (jancy)

cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

include (CMakeUtils)

#..............................................................................
#
# Standard settings
#

include (CMakeStdSettings)

create_std_settings ()

message (STATUS "Target CPU:                 ${TARGET_CPU}")

if (HAS_BUILD_TYPE_SETTING)
	message (STATUS "Build type:                 ${CMAKE_BUILD_TYPE}")
endif ()

message (STATUS "C++ flags (Debug):         ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
message (STATUS "C++ flags (Release):       ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")

get_directory_property (_CXX_DEFINITIONS COMPILE_DEFINITIONS)

if (_CXX_DEFINITIONS)
	message (STATUS "C++ definitions:            ${_CXX_DEFINITIONS}")
endif ()

#..............................................................................
#
# Paths to libs & tools
#

ensure_file_exists (
	${CMAKE_MODULE_PATH}/CMakePaths.cmake
	${CMAKE_MODULE_PATH}/CMakePaths-template.cmake
	)

include (CMakePaths)

message (STATUS "Path to RAGEL executable:   ${PATH_RAGEL_EXE}")
message (STATUS "Path to LLVM cmake modules: ${PATH_LLVM_CMAKE_DIR}")
message (STATUS "Path to LLVM includes:      ${PATH_LLVM_INC_DIR}")
message (STATUS "Path to LLVM includes (2):  ${PATH_LLVM_INC_DIR_2}")
message (STATUS "Path to LLVM libraries:     ${PATH_LLVM_LIB_DIR}")
message (STATUS "Path to AXL includes:       ${PATH_AXL_INC_DIR}")
message (STATUS "Path to AXL libraries:      ${PATH_AXL_LIB_DIR}")
message (STATUS "Path to Gepard includes:    ${PATH_GEPARD_INC_DIR}")
message (STATUS "Path to Gepard executable:  ${PATH_GEPARD_LLKC_EXE}")

include ("${PATH_LLVM_CMAKE_DIR}/LLVMConfig.cmake")

if (NOT "${PATH_QT_CMAKE_DIR}" STREQUAL "")
	message (STATUS "Path to QT cmake modules:   ${PATH_QT_CMAKE_DIR}")

	unset (Qt5Core_DIR    CACHE)
	unset (Qt5Gui_DIR     CACHE)
	unset (Qt5Widgets_DIR CACHE)

	include ("${PATH_QT_CMAKE_DIR}/Qt5Core/Qt5CoreConfig.cmake")
	include ("${PATH_QT_CMAKE_DIR}/Qt5Widgets/Qt5WidgetsConfig.cmake")
	set (QT_FOUND TRUE)
endif ()

#..............................................................................
#
# main
#

set (JNC_ROOT_DIR   ${CMAKE_CURRENT_LIST_DIR})
set (JNC_INC_DIR    ${JNC_ROOT_DIR}/include)

option (
	JNC_USE_PCH
	"Use precompiled headers when building Jancy project"
	ON
	)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

include_directories (${JNC_INC_DIR})

enable_testing ()

add_subdirectory (
	samples
	${CMAKE_CURRENT_BINARY_DIR}/jnc_demos
	)

add_subdirectory (src)
add_subdirectory (tests)

#..............................................................................
