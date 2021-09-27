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

option(
	BUILD_JNC_TESTS
	"Build Jancy tests"
	ON
)

if(NOT BUILD_JNC_TESTS)
	return()
endif()

add_subdirectory(jnc_test_abi)
add_subdirectory(jnc)
add_subdirectory(ioninja)

#...............................................................................
