# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2016. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

project (jancy)

cmake_minimum_required (VERSION 2.8)

include (cmake/axl_cmake.cmake NO_POLICY_SCOPE)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (JANCY_ROOT_DIR     ${CMAKE_CURRENT_LIST_DIR})
set (JANCY_LIB_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
set (JANCY_BIN_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set (JANCY_DOC_DIR      ${CMAKE_CURRENT_BINARY_DIR}/doc)
set (JANCY_PACKAGE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/package)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

add_subdirectory (cmake)

include (${CMAKE_CURRENT_BINARY_DIR}/cmake/jancy_config.cmake)

add_subdirectory (src)
add_subdirectory (doc)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/samples/CMakeLists.txt")
	add_subdirectory (samples)
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
