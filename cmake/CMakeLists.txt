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

include(${JANCY_ROOT_DIR}/version.cmake)

string(REGEX REPLACE "[a-z]+" ".." JANCY_INSTALL_ROOT_SUBDIR ${JANCY_INSTALL_CMAKE_SUBDIR})

configure_file(
	jancy_config_build.cmake.in
	jancy_config.cmake
	@ONLY
	)

configure_file(
	jancy_config_install.cmake.in
	jancy_config_install.cmake
	@ONLY
	)

# make JANCY_CMAKE_DIR available to any project within the same source tree

set(
	JANCY_CMAKE_DIR
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	CACHE INTERNAL
	"In-source-tree Jancy CMake directories"
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES import_jancy.cmake
	DESTINATION ${JANCY_INSTALL_CMAKE_SUBDIR}
	)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/jancy_config_install.cmake
	RENAME jancy_config.cmake
	DESTINATION ${JANCY_INSTALL_CMAKE_SUBDIR}
	)

install(
	FILES ${JANCY_ROOT_DIR}/version.cmake
	RENAME jancy_version.cmake
	DESTINATION ${JANCY_INSTALL_CMAKE_SUBDIR}
	)

#...............................................................................
