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

add_subdirectory (jnc_ct)
add_subdirectory (jnc_rt)
add_subdirectory (jnc_api)
add_subdirectory (jnc_ext)

option (
	BUILD_JNC_APP
	"Build jancy command line application"
	ON
	)

if (BUILD_JNC_APP)
	add_subdirectory (jnc_app)
endif ()

#...............................................................................
