# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2014. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

configure_file (
	jancy_config.cmake.in
	jancy_config.cmake
	@ONLY
	)

if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/../paths.cmake)
	configure_file (
		paths.cmake.template
		${CMAKE_CURRENT_LIST_DIR}/../paths.cmake
		COPYONLY
		)
endif ()

include (${CMAKE_CURRENT_BINARY_DIR}/jancy_config.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/../paths.cmake)

#..............................................................................
