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

push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

configure_file (
	Version.h.in
	${GEN_DIR}/Version.h
	)

set (
	GEN_H_LIST
	${GEN_DIR}/Version.h
	)

pop (CMAKE_CURRENT_BINARY_DIR)

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
# jncc jancy compiler
#

include_directories (
	${LLVM_INC_DIR}
	${LLVM_INC_DIR_2}
	${AXL_INC_DIR}
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
	MCDisassembler
	)

add_executable (
	jncc
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
	)

set_target_properties (
	jncc
	PROPERTIES
	OUTPUT_NAME jancy
	)

target_link_libraries (
	jncc
	jnc
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (WIN32)
	target_link_libraries (
		jncc
		ws2_32
		)
elseif (UNIX)
	target_link_libraries (
		jncc
		rt
		pthread
		)
endif ()

set_pch (
	jncc
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................

# macro for running jancy compiler on specified list of test source files

macro (
add_jncc_tests
	_FLAGS
	_DIR
	# ...
	)

	set (_JNCC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/jancy)
	set (_FILE_NAME_LIST ${ARGN})
	string (REPLACE " " ";" _FLAG_LIST ${_FLAGS})

	foreach (_FILE_NAME ${_FILE_NAME_LIST})
		add_test (
			NAME ${_FILE_NAME}
			WORKING_DIRECTORY ${_DIR}
			COMMAND ${_JNCC} ${_FLAG_LIST} ${_FILE_NAME}
			)
	endforeach ()
endmacro ()

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# test against official samples

if (UNIX)
	set (_FLAGS "--mcjit --run")
else ()
	set (_FLAGS "--run")
endif ()

add_jncc_tests (
	${_FLAGS}
	${JANCY_SAMPLE_DIR}
	${JANCY_SAMPLE_LIST}
	)

# later add more test files

#..............................................................................
