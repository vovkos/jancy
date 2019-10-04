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
	SAMPLE_JNC_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	*.jnc
	)

source_group(
	jnc
	FILES
	${SAMPLE_JNC_LIST}
	)

if(${BUILD_JNC_APP})
	add_jancy_tests(
		NAME_PREFIX "jnc-sample-"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		TESTS ${SAMPLE_JNC_LIST}
		)
endif()

#...............................................................................
