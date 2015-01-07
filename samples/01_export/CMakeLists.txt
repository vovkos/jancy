# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	ApiSlots.h
	MyLib.h
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

source_group (
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
	)	

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc & gen folder
#

# put script next to executable and track changes to the original file

set (
	SCRIPT_JNC
	${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX}/script.jnc
	)

add_custom_command (
	OUTPUT ${SCRIPT_JNC}
	MAIN_DEPENDENCY script.jnc.in
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_LIST_DIR}/script.jnc.in
		"${SCRIPT_JNC}"
	)

set (
	JNC_LIST
	script.jnc.in
	)

set (
	GEN_LIST
	${SCRIPT_JNC}
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
	)	

source_group (
	gen
	FILES
	${GEN_LIST}
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
	${JNC_LIST}
	${GEN_LIST}
	)

set_target_properties (
	01_export
	PROPERTIES
	FOLDER samples
	)
	
target_link_libraries (
	01_export
	jnc
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

set_pch (
	01_export
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
