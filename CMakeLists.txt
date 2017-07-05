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

cmake_minimum_required (VERSION 3.3)

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

set (JANCY_INSTALL_BIN_SUBDIR bin)
set (JANCY_INSTALL_LIB_SUBDIR lib)

if (WIN32)
	set (JANCY_INSTALL_INC_SUBDIR     include)
	set (JANCY_INSTALL_CMAKE_SUBDIR   cmake)
	set (JANCY_INSTALL_JNCX_SUBDIR    bin)
	set (JANCY_INSTALL_TOOL_SUBDIR    tools)
	set (JANCY_INSTALL_SAMPLE_SUBDIR  samples)
	set (JANCY_INSTALL_SPHINX_SUBDIR  sphinx)
	set (JANCY_INSTALL_DOC_SUBDIR     doc)
	set (JANCY_INSTALL_LICENSE_SUBDIR license)
else ()
	set (JANCY_INSTALL_INC_SUBDIR     include/${CMAKE_PROJECT_NAME})
	set (JANCY_INSTALL_CMAKE_SUBDIR   lib/cmake/${CMAKE_PROJECT_NAME})
	set (JANCY_INSTALL_JNCX_SUBDIR    share/${CMAKE_PROJECT_NAME}/extensions)
	set (JANCY_INSTALL_TOOL_SUBDIR    share/${CMAKE_PROJECT_NAME}/tools)
	set (JANCY_INSTALL_SAMPLE_SUBDIR  share/${CMAKE_PROJECT_NAME}/samples)
	set (JANCY_INSTALL_SPHINX_SUBDIR  share/${CMAKE_PROJECT_NAME}/sphinx)
	set (JANCY_INSTALL_DOC_SUBDIR     share/${CMAKE_PROJECT_NAME}/doc)
	set (JANCY_INSTALL_LICENSE_SUBDIR share/${CMAKE_PROJECT_NAME}/license)
endif ()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

add_subdirectory (cmake)

include (${CMAKE_CURRENT_BINARY_DIR}/cmake/jancy_config.cmake)

add_subdirectory (src)
add_subdirectory (doc)
add_subdirectory (samples)
add_subdirectory (test)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install (
	DIRECTORY include/
	DESTINATION ${JANCY_INSTALL_INC_SUBDIR}
	)

install (
	DIRECTORY tools/
	DESTINATION ${JANCY_INSTALL_TOOL_SUBDIR}
	)

install (
	DIRECTORY samples/
	DESTINATION ${JANCY_INSTALL_SAMPLE_SUBDIR}
	PATTERN "CMakeLists.txt" EXCLUDE
	)

install (
	DIRECTORY sphinx/
	DESTINATION ${JANCY_INSTALL_SPHINX_SUBDIR}
	PATTERN "*.pyc" EXCLUDE
	PATTERN "__pycache__" EXCLUDE
	)

install (
	DIRECTORY license/
	DESTINATION ${JANCY_INSTALL_LICENSE_SUBDIR}
	)

#...............................................................................

set (CPACK_PACKAGE_VERSION_MAJOR ${JANCY_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${JANCY_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${JANCY_VERSION_REVISION})

if (WIN32)
	set (CPACK_SOURCE_GENERATOR 7Z)
else ()
	set (CPACK_SOURCE_GENERATOR TXZ)
endif ()

set (CPACK_SOURCE_IGNORE_FILES "/[.];[.]user$;/build/;/ci/;/paths.cmake$")

string (TOLOWER ${CMAKE_SYSTEM_NAME} OS_NAME)

set (CPACK_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${OS_NAME}-${JANCY_VERSION_FILE_SUFFIX}-${TARGET_CPU})
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-src-${JANCY_VERSION_FILE_SUFFIX})

include (CPack)

#...............................................................................
