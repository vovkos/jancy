# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (jnc_core)
add_subdirectory (jnc_std)

option (
	BUILD_JNC_IO
	"Build jancy IO extension library"
	ON
	)

if (BUILD_JNC_IO)
	add_subdirectory (jnc_io)
endif ()

option (
	BUILD_JNC_APP
	"Build jancy command line application"
	ON
	)

if (BUILD_JNC_APP)
	add_subdirectory (jnc_app)
endif ()


#..............................................................................
