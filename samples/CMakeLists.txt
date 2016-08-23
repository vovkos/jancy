# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (00_jnc)

option (
	BUILD_SAMPLE_01_EMBED_C
	"Build sample 01_embed_c"
	ON
	)

if (BUILD_SAMPLE_01_EMBED_C)
	add_subdirectory (01_embed_c)
endif ()

option (
	BUILD_SAMPLE_02_EMBED_CPP
	"Build sample 02_embed_cpp"
	ON
	)

if (BUILD_SAMPLE_02_EMBED_CPP)
	add_subdirectory (02_embed_cpp)
endif ()

if (${QT_FOUND})
	option (
		BUILD_SAMPLE_03_DIALOG
		"Build sample 03_dialog (QT bindings)"
		ON
		)

	if (BUILD_SAMPLE_03_DIALOG)
		add_subdirectory (03_dialog)
	endif ()
endif ()

#..............................................................................
