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
add_subdirectory (jnc_ext) # ext BEFORE everything api, app & dll (we need BUILD_JNC_IO_USB)
add_subdirectory (jnc_api)

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

option (
	BUILD_JNC_STDLIB
	"Build jnc_stdlib.jncx (for IDE)"
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

if (BUILD_JNC_STDLIB)
	add_subdirectory (jnc_stdlib)
endif ()

#...............................................................................
