# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

project_virtual_folder (jncc)

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	jncc.h
	jncc_CmdLine.h
	jncc_OutStream.h
	)

set (
	APP_CPP_LIST
	jncc.cpp
	jncc_CmdLine.cpp
	jncc_Compile.cpp
	jncc_RunFunction.cpp
	jncc_Server.cpp
	jncc_OutStream.cpp
	)

source_virtual_folder (
	app
	${APP_H_LIST}
	${APP_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H   pch.h)
set (PCH_CPP pch.cpp)

source_virtual_folder (
	pch
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
#
# jncc jancy compiler
#

include_directories (
	${PATH_AXL_INC_DIR}
	${PATH_LLVM_INC_DIR}
	${PATH_LLVM_INC_DIR_2}
	)

link_directories (
	${PATH_AXL_LIB_DIR}
	${PATH_LLVM_LIB_DIR}
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

if (AXL_USE_PCH)
	set_pch (
		jncc
		${PCH_H}
		${PCH_CPP}
		)
endif ()

#..............................................................................

# macro for running jancy compiler on specified list of test source files

macro (
add_jncc_tests
	_FLAGS
	_DIR
	# ...
	)

	set (_JNCC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/jncc)
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

# test against official demos

if (UNIX)
	set (_FLAGS "--mcjit --run")
else ()
	set (_FLAGS "--run")
endif ()

add_jncc_tests (
	${_FLAGS}
	${AXL_JANCY_DEMOS_DIR}
	${AXL_JANCY_DEMOS_FILE_NAME_LIST}
	)

# later add more test files

#..............................................................................
