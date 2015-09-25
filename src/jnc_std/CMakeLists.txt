# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# stdlib folder
#

set (
	LIB_H_LIST
	jnc_Error.h
	jnc_Buffer.h
	jnc_String.h
	jnc_HashTable.h
	jnc_List.h
	jnc_StdLib.h
	)

set (
	LIB_CPP_LIST
	jnc_Error.cpp
	jnc_Buffer.cpp
	jnc_String.cpp
	jnc_HashTable.cpp
	jnc_List.cpp
	jnc_StdLib.cpp
	)

set (
	LIB_JNC_LIST
	jnc_Buffer.jnc
	jnc_String.jnc
	jnc_HashTable.jnc
	jnc_List.jnc
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
	jnc_Buffer.jnc.cpp
	jnc_Buffer.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_String.jnc.cpp
	jnc_String.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_HashTable.jnc.cpp
	jnc_HashTable.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_List.jnc.cpp
	jnc_List.jnc
	${JNC2CPP_PL}
	)
	
pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_CPP_LIST
	${GEN_DIR}/jnc_Buffer.jnc.cpp
	${GEN_DIR}/jnc_String.jnc.cpp
	${GEN_DIR}/jnc_HashTable.jnc.cpp
	${GEN_DIR}/jnc_List.jnc.cpp
	)

exclude_from_build (${GEN_CPP_LIST})  # include "*.jnc.cpp" manually

source_group (
	gen
	FILES
	${GEN_CPP_LIST}
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
	${LLVM_INC_DIR_2}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	${CMAKE_CURRENT_LIST_DIR}
	)

add_library (
	jnc_std
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	${GEN_CPP_LIST}
	)

set_pch (
	jnc_std
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
