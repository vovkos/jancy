#..............................................................................

axl_find_file (
	_CONFIG_CMAKE
	jancy_config.cmake
	${JANCY_CMAKE_DIR}
	)

if (_CONFIG_CMAKE)
	include (${_CONFIG_CMAKE})

	message (STATUS "Jancy paths:")
	axl_message ("    CMake files:" "${JANCY_CMAKE_DIR}")
	axl_message ("    Includes:"    "${JANCY_INC_DIR}")
	axl_message ("    Libraries:"   "${JANCY_LIB_DIR}")

	set (JANCY_FOUND TRUE)
else ()
	set (JANCY_FOUND FALSE)
endif ()

#..............................................................................
