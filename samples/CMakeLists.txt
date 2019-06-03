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
	BUILD_C_SAMPLES
	"Build C samples"
	ON
	)

option(
	BUILD_CPP_SAMPLES
	"Build C++ samples"
	ON
	)

if(BUILD_C_SAMPLES)
	add_subdirectory(jnc_sample_01_export_c)
	add_subdirectory(jnc_sample_04_pass_c)
endif()

if(BUILD_CPP_SAMPLES)
	add_subdirectory(jnc_sample_02_export_cpp)
	add_subdirectory(jnc_sample_05_pass_cpp)

	if(${QT_FOUND})
		add_subdirectory(jnc_sample_03_dialog)
	endif()
endif()

#...............................................................................
