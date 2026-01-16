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

set(
	LIB_H_LIST
	jnc_std_Guid.h
	jnc_std_Error.h
	jnc_std_String.h
	jnc_std_Buffer.h
	jnc_std_Array.h
	jnc_std_List.h
	jnc_std_Map.h
	jnc_std_HashTable.h
	jnc_std_RbTree.h
	jnc_std_StdLib.h
)

set(
	LIB_CPP_LIST
	jnc_std_Guid.cpp
	jnc_std_Error.cpp
	jnc_std_String.cpp
	jnc_std_Buffer.cpp
	jnc_std_Array.cpp
	jnc_std_List.cpp
	jnc_std_Map.cpp
	jnc_std_HashTable.cpp
	jnc_std_RbTree.cpp
	jnc_std_StdLib.cpp
)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set(
	JNC_LIST
	jnc/std_globals.jnc
	jnc/std_Guid.jnc
	jnc/std_Error.jnc
	jnc/std_String.jnc
	jnc/std_Buffer.jnc
	jnc/std_Array.jnc
	jnc/std_List.jnc
	jnc/std_MapEntry.jnc
	jnc/std_HashTable.jnc
	jnc/std_RbTree.jnc
	jnc/stdt_Array.jnc
	jnc/stdt_Iterator.jnc
	jnc/stdt_List.jnc
	jnc/stdt_BoxList.jnc
	jnc/stdt_Map.jnc
	jnc/stdt_HashTable.jnc
	jnc/stdt_BinTree.jnc
	jnc/stdt_RbTree.jnc
	jnc/stdt_Operator.jnc
)

source_group(
	jnc
	FILES
	${JNC_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

foreach(_JNC ${JNC_LIST})
	get_filename_component(_NAME ${_JNC} NAME)

	add_perl_step(
		${_NAME}.cpp
		${_JNC}
		${FILE2LITERAL_PL}
	)

	list(APPEND GEN_JNC_CPP_LIST ${GEN_DIR}/${_NAME}.cpp)
endforeach()

axl_pop(CMAKE_CURRENT_BINARY_DIR)

axl_exclude_from_build(${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

source_group(
	gen
	FILES
	${GEN_JNC_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H     pch.h)
set(PCH_O_CPP pch_o.cpp)

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_std library
#

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
)

add_library(
	jnc_std
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
)

target_precompile_headers(
	jnc_std
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_std
	PROPERTIES
	FOLDER extensions
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# object library to be re-used as sources for the jancy shared/dylib

if(UNIX)
	add_library(
		jnc_std_o OBJECT
		${PCH_H}
		${PCH_O_CPP}
		${LIB_H_LIST}
		${LIB_CPP_LIST}
		${JNC_LIST}
		${GEN_JNC_CPP_LIST}
	)

	target_compile_definitions(
		jnc_std_o
		PUBLIC
		-D_JNC_OBJECT_LIB=1
	)

	target_precompile_headers(
		jnc_std_o
		PRIVATE
	${PCH_H}
		${PCH_O_CPP}
	)

	set_target_properties(
		jnc_std_o
		PROPERTIES
		FOLDER extensions
	)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_std
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

#...............................................................................
