#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

project (jancy)

cmake_minimum_required (VERSION 2.8)

include (cmake/axl_cmake.cmake NO_POLICY_SCOPE)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (JANCY_ROOT_DIR     ${CMAKE_CURRENT_LIST_DIR})
set (JANCY_SRC_DIR      ${CMAKE_CURRENT_LIST_DIR}/src)
set (JANCY_LIB_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
set (JANCY_BIN_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set (JANCY_DOC_DIR      ${CMAKE_CURRENT_BINARY_DIR}/doc)
set (JANCY_PACKAGE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/package)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX_0})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX_0})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${JANCY_LIB_BASE_DIR}/${CONFIGURATION_SUFFIX_0})

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

#...............................................................................
