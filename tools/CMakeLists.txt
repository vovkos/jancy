# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

option (
	BUILD_TOOL_JNCX_INFO
	"Build jncx-info tool"
	ON
	)

if (BUILD_TOOL_JNCX_INFO)
	add_subdirectory (jncx-info)
endif ()

#..............................................................................
