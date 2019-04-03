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

add_subdirectory(jnc)

option(
	BUILD_JNC_SAMPLE_01_EMBED_C
	"Build sample jnc_sample_01_embed_c"
	ON
	)

if(BUILD_JNC_SAMPLE_01_EMBED_C)
	add_subdirectory(jnc_sample_01_embed_c)
endif()

option(
	BUILD_JNC_SAMPLE_02_EMBED_CPP
	"Build sample jnc_sample_02_embed_cpp"
	ON
	)

if(BUILD_JNC_SAMPLE_02_EMBED_CPP)
	add_subdirectory(jnc_sample_02_embed_cpp)
endif()

if(${QT_FOUND})
	option(
		BUILD_JNC_SAMPLE_03_DIALOG
		"Build sample jnc_sample_03_dialog(QT bindings)"
		ON
		)

	if(BUILD_JNC_SAMPLE_03_DIALOG)
		add_subdirectory(jnc_sample_03_dialog)
	endif()
endif()

#...............................................................................
