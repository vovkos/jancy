#..............................................................................

if ("${JANCY_CMAKE_DIR_2}" STREQUAL "")
	set (JANCY_FOUND FALSE)
	message (STATUS "Jancy:                      <not-found>")
else ()
	include (${JANCY_CMAKE_DIR_2}/jancy_config.cmake)	

	set (JANCY_FOUND TRUE)
	message (STATUS "Path to Jancy cmake files:  ${JANCY_CMAKE_DIR}")
	message (STATUS "Path to Jancy includes:     ${JANCY_INC_DIR}")
	message (STATUS "Path to Jancy libraries:    ${JANCY_LIB_DIR}")
endif ()

#..............................................................................
