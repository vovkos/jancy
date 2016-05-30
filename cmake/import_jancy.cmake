#..............................................................................

axl_find_file (
	_CONFIG_CMAKE
	jancy_config.cmake
	${JANCY_CMAKE_DIR}
	)

if (_CONFIG_CMAKE)
	include (${_CONFIG_CMAKE})

	message (STATUS "Path definitions for Jancy:")
	message (STATUS "    Jancy cmake files: ${JANCY_CMAKE_DIR}")
	message (STATUS "    Jancy includes:    ${JANCY_INC_DIR}")
	message (STATUS "    Jancy libraries:   ${JANCY_LIB_DIR}")

	set (JANCY_FOUND TRUE)
else ()
	set (JANCY_FOUND FALSE)
endif ()

#..............................................................................
