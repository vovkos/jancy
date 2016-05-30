# to make use of AXL cmake build infrastructure, copy this file into 
# your project directory and include it in master CMakeLists.txt like this:

# include (axl_cmake.cmake NO_POLICY_SCOPE)

#..............................................................................

# make sure dependencies.cmake is where it's supposed to be

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/dependencies.cmake)
	message (FATAL_ERROR "dependencies.cmake not found (must be in the root dir)")
endif ()

# find paths.cmake in this directory or above

set (_DIR ${CMAKE_CURRENT_LIST_DIR})

while (TRUE)
	if (EXISTS ${_DIR}/paths.cmake)
		set (_PATHS_CMAKE ${_DIR}/paths.cmake)
		break ()
	endif ()
	
	get_filename_component (_PARENT_DIR "${_DIR}/.." ABSOLUTE)
	if (${_DIR} STREQUAL ${_PARENT_DIR})
		set (_PATHS_CMAKE paths.cmake-NOTFOUND)
		break ()
	endif ()	
	
	set (_DIR ${_PARENT_DIR})
endwhile ()

# if not found, generate a new one based on AXL_PATH_LIST from dependencies.cmake

if (NOT _PATHS_CMAKE)
	include (${CMAKE_SOURCE_DIR}/dependencies.cmake)

	axl_create_empty_setting_file (
		${CMAKE_SOURCE_DIR}/paths.cmake
		${AXL_PATH_LIST}
		)

	message (FATAL_ERROR "please fill the newly generated ${CMAKE_SOURCE_DIR}/paths.cmake")
endif ()

# include paths.cmake to get AXL_CMAKE_DIR

include (${_PATHS_CMAKE})

# find and include axl_init.cmake

if (NOT AXL_CMAKE_DIR)
	message (FATAL_ERROR "AXL_CMAKE_DIR not found (check your paths.cmake)")
endif ()

set (_AXL_INIT_CMAKE axl_init.cmake-NOTFOUND)

foreach (_DIR ${AXL_CMAKE_DIR})
	if (EXISTS ${_DIR}/axl_init.cmake)
		set (_AXL_INIT_CMAKE ${_DIR}/axl_init.cmake)
		break ()
	endif ()	
endforeach ()

if (NOT _AXL_INIT_CMAKE)
	message (FATAL_ERROR "axl_init.cmake not found (check AXL_CMAKE_DIR in your paths.cmake)")
endif ()

include (
	${_AXL_INIT_CMAKE}
	NO_POLICY_SCOPE
	)
	
# include paths again, this time ${CONFIGURATION_SUFFIX} is defined

include (${_PATHS_CMAKE})

# include dependecies.cmake, now all the settings are defined

include (${CMAKE_SOURCE_DIR}/dependencies.cmake)

# diagnostic printing

axl_print_std_settings ()

get_cmake_property (_VARIABLE_LIST VARIABLES)

string (REPLACE ";" "\$|^" _FILTER "^${AXL_PATH_LIST}\$")
axl_filter_list (_FILTERED_VARIABLE_LIST ${_FILTER} ${_VARIABLE_LIST})

message(STATUS "Path defintions in ${_PATHS_CMAKE}:")

axl_print_variable_list ("    " ${_FILTERED_VARIABLE_LIST})

# import modules (if dependencies.cmake defines any imports)

if (AXL_IMPORT_LIST)
	axl_import (${AXL_IMPORT_LIST})
endif ()

#..............................................................................
