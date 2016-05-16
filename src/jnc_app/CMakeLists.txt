# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	CmdLine.h
	Jnc.h
	OutStream.h
	version.h.in
	)

set (
	APP_CPP_LIST
	main.cpp
	CmdLine.cpp
	Jnc.cpp
	OutStream.cpp
	)

source_group (
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# res folder
#

set (
	RES_RC_LIST
	res.rc
	)

source_group (
	res
	FILES
	${RES_RC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

configure_file (
	version.h.in
	${GEN_DIR}/version.h
	)

set (
	GEN_H_LIST
	${GEN_DIR}/version.h
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

source_group (
	gen
	FILES
	${GEN_H_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H   pch.h)
set (PCH_CPP pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
#
# jnc_app jancy compiler
#

include_directories (
	${LLVM_INC_DIR}
	${LLVM_INC_DIR_2}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

link_directories (
	${LLVM_LIB_DIR}
	${AXL_LIB_DIR}
	)

llvm_map_components_to_libraries (
	LLVM_LIB_LIST
	jit
	native
	MCJIT
	)

add_executable (
	jnc_app
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
	)

axl_set_pch (
	jnc_app	
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_app
	PROPERTIES
	OUTPUT_NAME jancy
	)

target_link_libraries (
	jnc_app
	jnc_std
	jnc_sys
	jnc_core
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	${LLVM_LIB_LIST}
	)

if (WIN32)
	target_link_libraries (
		jnc_app
		ws2_32
		)
elseif (UNIX)
	target_link_libraries (
		jnc_app
		pthread
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_app
			rt
			)
	endif ()
endif ()

#..............................................................................

macro (
add_jancy_tests
	# ...
	)

	set (_FLAGS "--print-retval -I ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX}")

	if (UNIX)
		set (_FLAGS "${_FLAGS} --mcjit --debug-info")
	endif ()

	string (REPLACE " " ";" _FLAG_LIST ${_FLAGS})
	
	set (_FILE_NAME_LIST ${ARGN})
	set (_IS_DIR FALSE)
	set (_DIR)

	foreach (_FILE_NAME ${_FILE_NAME_LIST})
		if (${_IS_DIR})
			set (_DIR ${_FILE_NAME})
			set (_IS_DIR FALSE)
		elseif ("${_FILE_NAME}" STREQUAL "WORKING_DIRECTORY")
			set (_IS_DIR TRUE)
		elseif ("${_DIR}" STREQUAL "")
			add_test (
				NAME ${_FILE_NAME}
				COMMAND $<TARGET_FILE:jnc_app> ${_FLAG_LIST} ${_FILE_NAME}
				)
		else ()
			add_test (
				NAME ${_FILE_NAME}
				WORKING_DIRECTORY ${_DIR}
				COMMAND $<TARGET_FILE:jnc_app> ${_FLAG_LIST} ${_FILE_NAME}
				)
		endif ()
	endforeach ()
endmacro ()

#..............................................................................
