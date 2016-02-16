# to make use of AXL cmake build infrastructure, copy this file into cmake 
# directory of your project and include it in master CMakeLists.txt like this:

# include (cmake/axl_cmake.cmake NO_POLICY_SCOPE)

#..............................................................................

# create paths.cmake from template if it doesn't exists

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/paths.cmake)
	if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/paths.cmake.template)
		message (FATAL_ERROR "paths.cmake.template not found (should be next to axl_cmake.cmake)")
	endif ()
	
	configure_file (
		${CMAKE_CURRENT_LIST_DIR}/paths.cmake.template
		${CMAKE_SOURCE_DIR}/paths.cmake
		COPYONLY
		)
endif ()

# include paths.cmake to get AXL_CMAKE_DIR

include (${CMAKE_SOURCE_DIR}/paths.cmake)

if (AXL_CMAKE_DIR)
	include (${AXL_CMAKE_DIR}/axl_init.cmake NO_POLICY_SCOPE)
else () 
	# try directory next to the source directory
	include (${CMAKE_SOURCE_DIR}/../axl/cmake/axl_init.cmake NO_POLICY_SCOPE)
endif ()

# include paths again, this time ${CONFIGURATION_SUFFIX} is defined

include (${CMAKE_SOURCE_DIR}/paths.cmake)

#..............................................................................
