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

axl_find_file (
	_CONFIG_CMAKE
	jancy_config.cmake
	${JANCY_CMAKE_DIR}
	)

if (_CONFIG_CMAKE)
	include (${_CONFIG_CMAKE})

	axl_message ("Jancy ${JANCY_VERSION_FULL} paths:")
	axl_message ("    CMake files:" "${JANCY_CMAKE_DIR}")
	axl_message ("    Includes:"    "${JANCY_INC_DIR}")
	axl_message ("    Libraries:"   "${JANCY_LIB_DIR}")
	axl_message ("    DLLs:"        "${JANCY_DLL_DIR}")
	axl_message ("    Sphinx exts:" "${JANCY_SPHINX_DIR}")
	axl_message ("    Jancy DLL:"   "${JANCY_DLL_NAME}")

	set (JANCY_FOUND TRUE)
else ()
	set (JANCY_FOUND FALSE)
endif ()

#...............................................................................
