# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2016. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

project (jancy)

cmake_minimum_required (VERSION 2.8)

include (cmake/axl_cmake.cmake NO_POLICY_SCOPE)

if (${GRACO_CMAKE_DIR})
	set (_GRACO_CMAKE_DIR ${GRACO_CMAKE_DIR})
else ()
	set (_GRACO_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../graco/cmake)
endif ()

set (
	AXL_IMPORT_DIR_LIST
	${AXL_IMPORT_DIR_LIST}
	${CMAKE_CURRENT_LIST_DIR}/cmake
	${_GRACO_CMAKE_DIR}
	)

axl_print_std_settings ()

axl_import (
	REQUIRED 
		perl
		ragel
		llvm
		axl
		graco
		7z
	OPTIONAL 
		qt
		libssh2
		openssl
		pcap
	)

if (UNIX AND NOT APPLE)
	axl_import (OPTIONAL libudev)
endif ()

#..............................................................................

set (JANCY_ROOT_DIR     ${CMAKE_CURRENT_LIST_DIR})
set (JANCY_LIB_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
set (JANCY_BIN_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set (JANCY_PACKAGE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/package)

add_subdirectory (cmake)

include (${CMAKE_CURRENT_BINARY_DIR}/cmake/jancy_config.cmake)

add_subdirectory (samples)
add_subdirectory (src)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/doc/CMakeLists.txt")
	add_subdirectory (doc)
endif ()	

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt")
	add_subdirectory (test)
endif ()	

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/tools/CMakeLists.txt")
	add_subdirectory (tools)
endif ()	
	
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/package/CMakeLists.txt")
	add_subdirectory (package)
endif ()	

#..............................................................................
