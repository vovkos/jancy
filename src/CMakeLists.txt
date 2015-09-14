# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (jnc_core)


option (
	BUILD_JNC_APP
	"Build jancy command line application"
	ON
	)

if (BUILD_JNC_APP)
	add_subdirectory (jnc_app)
endif ()


#..............................................................................
