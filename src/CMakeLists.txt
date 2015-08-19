# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (jnc)


option (
	BUILD_JNCC
	"Build jancy command line compiler"
	ON
	)

if (BUILD_JNCC)
	add_subdirectory (jncc)
endif ()


#..............................................................................
