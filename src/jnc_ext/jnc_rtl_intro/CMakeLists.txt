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
	jnc_rtl_Alias.h
	jnc_rtl_ArrayType.h
	jnc_rtl_AttributeBlock.h
	jnc_rtl_ClassType.h
	jnc_rtl_DerivableType.h
	jnc_rtl_EnumType.h
	jnc_rtl_Field.h
	jnc_rtl_Function.h
	jnc_rtl_FunctionType.h
	jnc_rtl_Module.h
	jnc_rtl_ModuleItem.h
	jnc_rtl_MemberBlock.h
	jnc_rtl_Namespace.h
	jnc_rtl_Property.h
	jnc_rtl_PropertyType.h
	jnc_rtl_StructType.h
	jnc_rtl_Type.h
	jnc_rtl_UnionType.h
	jnc_rtl_Variable.h
	jnc_rtl_IntrospectionLib.h
)

set(
	LIB_CPP_LIST
	jnc_rtl_Alias.cpp
	jnc_rtl_ArrayType.cpp
	jnc_rtl_AttributeBlock.cpp
	jnc_rtl_ClassType.cpp
	jnc_rtl_DerivableType.cpp
	jnc_rtl_EnumType.cpp
	jnc_rtl_Field.cpp
	jnc_rtl_Function.cpp
	jnc_rtl_FunctionType.cpp
	jnc_rtl_Module.cpp
	jnc_rtl_ModuleItem.cpp
	jnc_rtl_MemberBlock.cpp
	jnc_rtl_Namespace.cpp
	jnc_rtl_Property.cpp
	jnc_rtl_PropertyType.cpp
	jnc_rtl_StructType.cpp
	jnc_rtl_Type.cpp
	jnc_rtl_UnionType.cpp
	jnc_rtl_Variable.cpp
	jnc_rtl_IntrospectionLib.cpp
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
	jnc/jnc_intro.jnc
	jnc/jnc_Alias.jnc
	jnc/jnc_ArrayType.jnc
	jnc/jnc_AttributeBlock.jnc
	jnc/jnc_ClassType.jnc
	jnc/jnc_DerivableType.jnc
	jnc/jnc_EnumType.jnc
	jnc/jnc_Field.jnc
	jnc/jnc_Function.jnc
	jnc/jnc_FunctionType.jnc
	jnc/jnc_Module.jnc
	jnc/jnc_ModuleItem.jnc
	jnc/jnc_MemberBlock.jnc
	jnc/jnc_Namespace.jnc
	jnc/jnc_OpKind.jnc
	jnc/jnc_Property.jnc
	jnc/jnc_PropertyType.jnc
	jnc/jnc_StructType.jnc
	jnc/jnc_Type.jnc
	jnc/jnc_UnionType.jnc
	jnc/jnc_Variable.jnc
)

source_group(
	jnc
	FILES
	${JNC_LIST}
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

#...............................................................................
#
# jnc_rtl_intro library
#

include_directories(
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_rt
	${GEN_DIR}
)

if(LINUX)
	include_directories(${OPENSSL_INC_DIR})
endif()

add_library(
	jnc_rtl_intro
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
)

target_precompile_headers(
	jnc_rtl_intro
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_rtl_intro
	PROPERTIES
	FOLDER extensions
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# object library to be re-used as sources for the jancy shared/dylib

if(UNIX)
	add_library(
		jnc_rtl_intro_o OBJECT
		${PCH_H}
		${PCH_O_CPP}
		${LIB_H_LIST}
		${LIB_CPP_LIST}
		${GEN_JNC_CPP_LIST}
	)

	target_compile_definitions(
		jnc_rtl_intro_o
		PUBLIC
		-D_JNC_OBJECT_LIB=1
	)

	target_precompile_headers(
		jnc_rtl_intro_o
		PRIVATE
	${PCH_H}
		${PCH_O_CPP}
	)

	set_target_properties(
		jnc_rtl_intro_o
		PROPERTIES
		FOLDER extensions
	)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_rtl_intro
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

#...............................................................................
