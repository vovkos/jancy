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

cmake_minimum_required(VERSION 3.16)

project(jancy)

include(cmake/axl_cmake.cmake NO_POLICY_SCOPE)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set(JANCY_ROOT_DIR       ${CMAKE_CURRENT_LIST_DIR})
set(JANCY_SRC_DIR        ${CMAKE_CURRENT_LIST_DIR}/src)
set(JANCY_BUILD_INC_DIR  ${CMAKE_CURRENT_BINARY_DIR}/include)
set(JANCY_LIB_BASE_DIR   ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(JANCY_BIN_BASE_DIR   ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(JANCY_DOC_DIR        ${CMAKE_CURRENT_BINARY_DIR}/doc)
set(JANCY_PACKAGE_DIR    ${CMAKE_CURRENT_BINARY_DIR}/package)

if(WIN32)
	set(JANCY_DLL_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
else()
	set(JANCY_DLL_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SCG})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${JANCY_DLL_BASE_DIR}/${CONFIGURATION_SCG})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${JANCY_LIB_BASE_DIR}/${CONFIGURATION_SCG})

set(JANCY_INSTALL_BIN_SUBDIR bin)
set(JANCY_INSTALL_LIB_SUBDIR lib)

if(WIN32)
	set(JANCY_INSTALL_INC_SUBDIR     include)
	set(JANCY_INSTALL_CMAKE_SUBDIR   cmake)
	set(JANCY_INSTALL_DLL_SUBDIR     bin)
	set(JANCY_INSTALL_JNCX_SUBDIR    bin)
	set(JANCY_INSTALL_TOOL_SUBDIR    tools)
	set(JANCY_INSTALL_SAMPLE_SUBDIR  samples)
	set(JANCY_INSTALL_SPHINX_SUBDIR  sphinx)
	set(JANCY_INSTALL_DOC_SUBDIR     doc)
	set(JANCY_INSTALL_LICENSE_SUBDIR license)
else()
	set(JANCY_INSTALL_INC_SUBDIR     include/${PROJECT_NAME})
	set(JANCY_INSTALL_CMAKE_SUBDIR   lib/cmake/${PROJECT_NAME})
	set(JANCY_INSTALL_DLL_SUBDIR     lib)
	set(JANCY_INSTALL_JNCX_SUBDIR    share/${PROJECT_NAME}/extensions)
	set(JANCY_INSTALL_TOOL_SUBDIR    share/${PROJECT_NAME}/tools)
	set(JANCY_INSTALL_SAMPLE_SUBDIR  share/${PROJECT_NAME}/samples)
	set(JANCY_INSTALL_SPHINX_SUBDIR  share/${PROJECT_NAME}/sphinx)
	set(JANCY_INSTALL_DOC_SUBDIR     share/${PROJECT_NAME}/doc)
	set(JANCY_INSTALL_LICENSE_SUBDIR share/${PROJECT_NAME}/license)

	if(APPLE)
		set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
	else()
		set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
	endif()
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

add_subdirectory(cmake)

include(${CMAKE_CURRENT_BINARY_DIR}/cmake/jancy_config.cmake)

configure_file(
	${JANCY_ROOT_DIR}/include/jnc_Config.h.in
	${JANCY_BUILD_INC_DIR}/jnc_Config.h
)

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(samples)
add_subdirectory(test)
add_subdirectory(doc)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	DIRECTORY include/
	DESTINATION ${JANCY_INSTALL_INC_SUBDIR}
	PATTERN "*.h"
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/include/jnc_Config.h
	DESTINATION ${JANCY_INSTALL_INC_SUBDIR}
)

install(
	DIRECTORY tools/
	DESTINATION ${JANCY_INSTALL_TOOL_SUBDIR}
)

install(
	DIRECTORY samples/
	DESTINATION ${JANCY_INSTALL_SAMPLE_SUBDIR}
	PATTERN "CMakeLists.txt" EXCLUDE
)

install(
	DIRECTORY sphinx/
	DESTINATION ${JANCY_INSTALL_SPHINX_SUBDIR}
	PATTERN "*.pyc" EXCLUDE
	PATTERN "__pycache__" EXCLUDE
)

install(
	DIRECTORY license/
	DESTINATION ${JANCY_INSTALL_LICENSE_SUBDIR}
)

#...............................................................................

set(CPACK_PACKAGE_VERSION_MAJOR ${JANCY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${JANCY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${JANCY_VERSION_REVISION})

if(WIN32)
	set(CPACK_SOURCE_GENERATOR 7Z)
else()
	set(CPACK_SOURCE_GENERATOR TXZ)
endif()

set(CPACK_SOURCE_IGNORE_FILES "/[.];[.]user$;/build/;/ci/;/paths.cmake$")

set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-${JANCY_VERSION_FILE_SUFFIX}-src)

if(APPLE)
	set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${JANCY_VERSION_FILE_SUFFIX}-mac)
else()
	string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_NAME)
	set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${JANCY_VERSION_FILE_SUFFIX}-${OS_NAME}-${TARGET_CPU})
endif()

include(CPack)

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/ci/github-actions/print-package-file-name.cmake
	${CMAKE_CURRENT_BINARY_DIR}/print-package-file-name.cmake
	COPYONLY
)

#...............................................................................
