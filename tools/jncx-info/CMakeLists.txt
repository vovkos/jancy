# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2015. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	CmdLine.h
	version.h.in
	)

set (
	APP_CPP_LIST
	main.cpp
	CmdLine.cpp
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
	version.h.in
	${GEN_DIR}/version.h
	)

set (
	GEN_H_LIST
	${GEN_DIR}/version.h
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
# jncx-info 
#

include_directories (
	${LLVM_INC_DIR}
	${LLVM_INC_DIR_2}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

link_directories (
	${JANCY_LIB_DIR}
	${LLVM_LIB_DIR}
	${AXL_LIB_DIR}
	)

add_executable (
	jncx_info
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
	)

set_pch (
	jncx_info
	${PCH_H}
	${PCH_CPP}
	)

llvm_map_components_to_libraries (
	LLVM_LIB_LIST
	jit
	native
	MCJIT
	)

set_target_properties (
	jncx_info
	PROPERTIES
	OUTPUT_NAME jncx-info
	)

set_target_properties (
	jncx_info
	PROPERTIES
	FOLDER tools
	)
	
target_link_libraries (
	jncx_info
	jnc_core
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (UNIX)
	target_link_libraries (
		jncx_info
		dl
		pthread
		)
		
	if (NOT APPLE)
		target_link_libraries (
			jncx_info
			rt
			)
	endif ()
endif ()

#..............................................................................
