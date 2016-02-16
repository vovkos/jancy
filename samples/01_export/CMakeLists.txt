# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	MyLib.h
	MyLibGlobals.h
	TestClass.h
	TestStruct.h
	)

set (
	APP_CPP_LIST
	main.cpp
	MyLib.cpp
	TestClass.cpp
	TestStruct.cpp
	)

set (
	APP_JNC_LIST
	script.jnc
	)

source_group (
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
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
# 01_export sample
#

include_directories (
	${AXL_INC_DIR}
	${LLVM_INC_DIR}
	${LLVM_INC_DIR_2}
	${JANCY_INC_DIR}
	)

link_directories (
	${AXL_LIB_DIR}
	${LLVM_LIB_DIR}
	${JANCY_LIB_DIR}
	)

llvm_map_components_to_libraries (
	LLVM_LIB_LIST
	jit
	native
	MCJIT
	MCDisassembler
	)

add_executable (
	01_export
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	)

set_target_properties (
	01_export
	PROPERTIES
	FOLDER samples
	)
	
target_link_libraries (
	01_export
	jnc_std
	jnc_core
	axl_zip
	axl_fsm
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (UNIX AND NOT APPLE)
	target_link_libraries (
		01_export
		rt
		)
endif ()

axl_set_pch (
	01_export
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
