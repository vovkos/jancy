# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

project_virtual_folder (axl_jnc_tests)

#..............................................................................
#
# src folder
#

set (
	TESTS_JNC_LIST
	test1.jnc
	test2.jnc
	test3.jnc
	test4.jnc
	test5.jnc
	test6.jnc
	test7.jnc
	test8.jnc
	test9.jnc
	test10.jnc
	)

source_virtual_folder (
	src
	${TESTS_JNC_LIST}
	)

#..............................................................................
#
# axl_jnc_tests library
#

add_library (
	axl_jnc_tests
	EXCLUDE_FROM_ALL
	${TESTS_JNC_LIST}
	)

set_target_properties (
	axl_jnc_tests
	PROPERTIES
	FOLDER tests
	LINKER_LANGUAGE C
	)

#..............................................................................
