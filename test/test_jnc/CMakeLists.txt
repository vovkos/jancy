# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# jnc folder
#

file (
	GLOB
	TEST_JNC_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	test*.jnc
	)

source_group (
	jnc
	FILES
	${TEST_JNC_LIST}
	)

#..............................................................................
#
# test_jnc jancy script sample set
#

add_custom_target (
	test_jnc
	SOURCES
	${TEST_JNC_LIST}
	)

set_target_properties (
	test_jnc
	PROPERTIES
	FOLDER test
	)

add_jancy_tests (
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	${TEST_JNC_LIST}
	)
	
#..............................................................................
