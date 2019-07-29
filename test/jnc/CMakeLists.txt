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

file(
	GLOB
	TEST_JNC_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	test*.jnc
	)

if(NOT BUILD_JNC_IO_USB)
	list(
		REMOVE_ITEM
		TEST_JNC_LIST
		test82.jnc
		test83.jnc
		)
endif()

if (NOT WIN32)
	list(
		REMOVE_ITEM
		TEST_JNC_LIST
		test131.jnc   # sys.RegKey test
		)
endif()

source_group(
	jnc
	FILES
	${TEST_JNC_LIST}
	)

if(${BUILD_JNC_APP})
	add_jancy_tests(
		NAME_PREFIX "jnc-test-"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		${TEST_JNC_LIST}
		)
endif()

#...............................................................................
