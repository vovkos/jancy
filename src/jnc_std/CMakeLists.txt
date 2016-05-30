# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_std_Error.h
	jnc_std_Buffer.h
	jnc_std_String.h
	jnc_std_HashTable.h
	jnc_std_List.h
	jnc_std_StdLib.h
	jnc_std_StdLibGlobals.h
	)

set (
	LIB_CPP_LIST
	jnc_std_Error.cpp
	jnc_std_Buffer.cpp
	jnc_std_String.cpp
	jnc_std_HashTable.cpp
	jnc_std_List.cpp
	jnc_std_StdLib.cpp
	)

set (
	LIB_JNC_LIST
	jnc_std_globals.jnc
	jnc_std_Error.jnc
	jnc_std_Buffer.jnc
	jnc_std_String.jnc
	jnc_std_HashTable.jnc
	jnc_std_List.jnc
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

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	jnc_std_globals.jnc.cpp
	jnc_std_globals.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_std_Error.jnc.cpp
	jnc_std_Error.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_std_Buffer.jnc.cpp
	jnc_std_Buffer.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_std_String.jnc.cpp
	jnc_std_String.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_std_HashTable.jnc.cpp
	jnc_std_HashTable.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_std_List.jnc.cpp
	jnc_std_List.jnc
	${JNC2CPP_PL}
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/jnc_std_globals.jnc.cpp
	${GEN_DIR}/jnc_std_Error.jnc.cpp
	${GEN_DIR}/jnc_std_Buffer.jnc.cpp
	${GEN_DIR}/jnc_std_String.jnc.cpp
	${GEN_DIR}/jnc_std_HashTable.jnc.cpp
	${GEN_DIR}/jnc_std_List.jnc.cpp
	)

axl_exclude_from_build (${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

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
# jnc_std library
#

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

add_library (
	jnc_std
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

axl_set_pch (
	jnc_std
	${PCH_H}
	${PCH_CPP}
	)

if (GCC)
	axl_append_target_string_property (
		jnc_std
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

#..............................................................................
