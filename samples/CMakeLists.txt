# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (00_jnc)

option (
	BUILD_SAMPLE_01_EXPORT
	"Build sample 01_export"
	ON
	)

if (BUILD_SAMPLE_01_EXPORT)
	add_subdirectory (01_export)
endif ()


if (${QT_FOUND})
	option (
		BUILD_SAMPLE_02_DIALOG
		"Build sample 02_dialog"
		ON
		)

	if (BUILD_SAMPLE_02_DIALOG)
		add_subdirectory (02_dialog)
	endif ()
endif ()

#..............................................................................
