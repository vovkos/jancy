#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

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

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set (
	JNC_LIST
	jnc/std_globals.jnc
	jnc/std_Error.jnc
	jnc/std_Buffer.jnc
	jnc/std_String.jnc
	jnc/std_HashTable.jnc
	jnc/std_List.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	std_globals.jnc.cpp
	jnc/std_globals.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	std_Error.jnc.cpp
	jnc/std_Error.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	std_Buffer.jnc.cpp
	jnc/std_Buffer.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	std_String.jnc.cpp
	jnc/std_String.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	std_HashTable.jnc.cpp
	jnc/std_HashTable.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	std_List.jnc.cpp
	jnc/std_List.jnc
	${JNC2CPP_PL}
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/std_globals.jnc.cpp
	${GEN_DIR}/std_Error.jnc.cpp
	${GEN_DIR}/std_Buffer.jnc.cpp
	${GEN_DIR}/std_String.jnc.cpp
	${GEN_DIR}/std_HashTable.jnc.cpp
	${GEN_DIR}/std_List.jnc.cpp
	)

axl_exclude_from_build (${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

source_group (
	gen
	FILES
	${GEN_JNC_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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

#...............................................................................
#
# jnc_std library
#

include_directories (
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
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

axl_set_pch (
	jnc_std
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_std
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................