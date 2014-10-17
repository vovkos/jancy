# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2014. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

configure_file (
	jancy.cmake.in
	jancy.cmake
	@ONLY
	)

configure_file (
	jancy.cmake.in
	${CMAKE_CURRENT_LIST_DIR}/latest/jancy-${TARGET_CPU}.cmake
	@ONLY
	)

configure_file (
	jancy-latest.cmake.in
	${CMAKE_CURRENT_LIST_DIR}/latest/jancy.cmake
	@ONLY
	)

#..............................................................................
