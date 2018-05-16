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

option (
	BUILD_JNC_DLL
	"Build jancy shared library"
	ON
	)

if (BUILD_JNC_APP)
	add_subdirectory (jnc_app)
endif ()

if (BUILD_JNC_DLL)
	set (JNC_LIBRARY_OBJECT OBJECT)
	add_subdirectory (jnc_dll)
else ()
	set (JNC_LIBRARY_OBJECT)
endif ()

#...............................................................................
