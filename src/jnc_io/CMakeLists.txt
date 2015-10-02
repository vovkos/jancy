# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_Socket.h
	jnc_Serial.h
	jnc_File.h
	jnc_ioLib.h
	jnc_ioLibSlots.h
	)

set (
	LIB_CPP_LIST
	jnc_Socket.cpp
	jnc_Serial.cpp
	jnc_File.cpp
	jnc_ioLib.cpp
	)

set (
	LIB_JNC_LIST
	jnc_Socket.jnc
	jnc_Serial.jnc
	jnc_File.jnc
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

set (JNC2CPP_PL ${CMAKE_CURRENT_SOURCE_DIR}/../jnc_core/jnc2cpp.pl)

push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	jnc_Socket.jnc.cpp
	jnc_Socket.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_Serial.jnc.cpp
	jnc_Serial.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_File.jnc.cpp
	jnc_File.jnc
	${JNC2CPP_PL}
	)

pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/jnc_Socket.jnc.cpp
	${GEN_DIR}/jnc_Serial.jnc.cpp
	${GEN_DIR}/jnc_File.jnc.cpp
	)

exclude_from_build (${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

source_group (
	gen
	FILES
	${GEN_JNC_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
#
# jnc_io library
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
	MCDisassembler
	)

add_library (
	jnc_io 
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

set_pch (
	jnc_io
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_io
	PROPERTIES
	OUTPUT_NAME io
	SUFFIX .jncx
	)

target_link_libraries (
	jnc_io
	jnc_core
	jnc_std
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (WIN32)
	target_link_libraries (
		jnc_io
		ws2_32
		)
elseif (UNIX)
	target_link_libraries (
		jnc_io
		rt
		pthread
		)
endif ()

#..............................................................................
