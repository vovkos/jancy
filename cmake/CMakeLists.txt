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

configure_file (
	jancy_config.cmake.in
	jancy_config.cmake
	@ONLY
	)

# make JANCY_CMAKE_DIR available to any project within the same source tree

set (
	JANCY_CMAKE_DIR
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	CACHE INTERNAL
	"In-source-tree Jancy CMake directories"
	)

#...............................................................................
