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

#...............................................................................
