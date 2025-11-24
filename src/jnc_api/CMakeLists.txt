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
	${PROJECT_BINARY_DIR}/include/jnc_Config.h
	${JANCY_ROOT_DIR}/include/jnc_Def.h
	${JANCY_ROOT_DIR}/include/jnc_RuntimeStructs.h
	${JANCY_ROOT_DIR}/include/jnc_OpKind.h
	${JANCY_ROOT_DIR}/include/jnc_Error.h
	${JANCY_ROOT_DIR}/include/jnc_ModuleItem.h
	${JANCY_ROOT_DIR}/include/jnc_AttributeBlock.h
	${JANCY_ROOT_DIR}/include/jnc_Template.h
	${JANCY_ROOT_DIR}/include/jnc_Type.h
	${JANCY_ROOT_DIR}/include/jnc_DerivableType.h
	${JANCY_ROOT_DIR}/include/jnc_ArrayType.h
	${JANCY_ROOT_DIR}/include/jnc_FunctionType.h
	${JANCY_ROOT_DIR}/include/jnc_PropertyType.h
	${JANCY_ROOT_DIR}/include/jnc_EnumType.h
	${JANCY_ROOT_DIR}/include/jnc_StructType.h
	${JANCY_ROOT_DIR}/include/jnc_UnionType.h
	${JANCY_ROOT_DIR}/include/jnc_ClassType.h
	${JANCY_ROOT_DIR}/include/jnc_Alias.h
	${JANCY_ROOT_DIR}/include/jnc_Variable.h
	${JANCY_ROOT_DIR}/include/jnc_Function.h
	${JANCY_ROOT_DIR}/include/jnc_Property.h
	${JANCY_ROOT_DIR}/include/jnc_Namespace.h
	${JANCY_ROOT_DIR}/include/jnc_Variant.h
	${JANCY_ROOT_DIR}/include/jnc_String.h
	${JANCY_ROOT_DIR}/include/jnc_Promise.h
	${JANCY_ROOT_DIR}/include/jnc_Unit.h
	${JANCY_ROOT_DIR}/include/jnc_Module.h
	${JANCY_ROOT_DIR}/include/jnc_Runtime.h
	${JANCY_ROOT_DIR}/include/jnc_GcHeap.h
	${JANCY_ROOT_DIR}/include/jnc_ExtensionLib.h
	${JANCY_ROOT_DIR}/include/jnc_DynamicExtensionLibHost.h
	${JANCY_ROOT_DIR}/include/jnc_CallSite.h
	${JANCY_ROOT_DIR}/include/jnc_Construct.h
	${JANCY_ROOT_DIR}/include/jnc_StdMap.h
	${JANCY_ROOT_DIR}/include/jnc_StdHashTable.h
	${JANCY_ROOT_DIR}/include/jnc_StdBuffer.h
	${JANCY_ROOT_DIR}/include/jnc_DynamicLayout.h
	${JANCY_ROOT_DIR}/include/jnc_CodeAssist.h
	${JANCY_ROOT_DIR}/include/jnc_Capability.h
	${JANCY_ROOT_DIR}/include/jnc_WarningSuppression.h
)

set(
	LIB_DYNAMIC_CPP_LIST
	jnc_OpKind.cpp
	jnc_Error.cpp
	jnc_ModuleItem.cpp
	jnc_AttributeBlock.cpp
	jnc_Template.cpp
	jnc_Type.cpp
	jnc_DerivableType.cpp
	jnc_ArrayType.cpp
	jnc_FunctionType.cpp
	jnc_PropertyType.cpp
	jnc_EnumType.cpp
	jnc_StructType.cpp
	jnc_UnionType.cpp
	jnc_ClassType.cpp
	jnc_Alias.cpp
	jnc_Variable.cpp
	jnc_Function.cpp
	jnc_Property.cpp
	jnc_Namespace.cpp
	jnc_Variant.cpp
	jnc_String.cpp
	jnc_Promise.cpp
	jnc_Unit.cpp
	jnc_Module.cpp
	jnc_Runtime.cpp
	jnc_GcHeap.cpp
	jnc_StdHashTable.cpp
	jnc_StdRbTree.cpp
	jnc_StdBuffer.cpp
	jnc_DynamicLayout.cpp
	jnc_CodeAssist.cpp
	jnc_Capability.cpp
)

set(
	LIB_STATIC_CPP_LIST
	${LIB_DYNAMIC_CPP_LIST}
	jnc_DynamicExtensionLibHost.cpp
)

# we compile our own setjmp/longjmp on windows for two reasons:
# 1) since msvc16 2019, we can't take address of pure intrinsic setjmp
# 2) msvc longjmp does stack unwinding which can crash with llvm-jitted code

if(WIN32)
	if(${TARGET_CPU} STREQUAL "amd64")
		set(SETJMP_ASM jnc_setJmp_msc_amd64.asm)

		set_source_files_properties(
			${SETJMP_ASM}
			PROPERTIES
			LANGUAGE ASM_NASM
		)
	elseif(${TARGET_CPU} STREQUAL "arm64")
		set(SETJMP_ASM jnc_setJmp_msc_arm64.asm)

		set_source_files_properties(
			${SETJMP_ASM}
			PROPERTIES
			LANGUAGE ASM_MARMASM
		)
	endif()

	set(LIB_ASM_LIST ${SETJMP_ASM})
endif()

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_STATIC_CPP_LIST}
	${LIB_ASM_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H            pch.h)
set(PCH_STATIC_CPP   pch_static.cpp)
set(PCH_STATIC_O_CPP pch_static_o.cpp)
set(PCH_DYNAMIC_CPP  pch_dynamic.cpp)

set(
	PCH_H_LIST
	${PCH_H}
	${JANCY_ROOT_DIR}/include/jnc_Pch.h
)

source_group(
	pch
	FILES
	${PCH_H_LIST}
	${PCH_STATIC_CPP}
	${PCH_STATIC_O_CPP}
	${PCH_DYNAMIC_CPP}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_api static/static_o/dynamic libraries
#

include_directories(
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_rt
	${JANCY_SRC_DIR}/jnc_ext/jnc_rtl_core
	${JANCY_SRC_DIR}/jnc_ext/jnc_std
)

if(LINUX)
	include_directories(${OPENSSL_INC_DIR})
endif()

add_library(
	jnc_api_core
	${PCH_H_LIST}
	${PCH_STATIC_CPP}
	${LIB_H_LIST}
	${LIB_STATIC_CPP_LIST}
	${LIB_ASM_LIST}
	${LIB_OBJ_LIST}
)

target_precompile_headers(
	jnc_api_core
	PRIVATE
	${PCH_H}
	${PCH_STATIC_CPP}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# object library to be re-used as sources for the jancy shared/dylib

if(UNIX)
	add_library(
		jnc_api_core_o OBJECT
		${PCH_H_LIST}
		${PCH_STATIC_O_CPP}
		${LIB_H_LIST}
		${LIB_STATIC_CPP_LIST}
	)

	target_compile_definitions(
		jnc_api_core_o
		PUBLIC
		-D_JNC_OBJECT_LIB=1
	)

	target_precompile_headers(
		jnc_api_core_o
		PRIVATE
	${PCH_H}
		${PCH_STATIC_O_CPP}
	)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

add_library(
	jnc_api_ext
	${PCH_H_LIST}
	${PCH_DYNAMIC_CPP}
	${LIB_H_LIST}
	${LIB_DYNAMIC_CPP_LIST}
	${LIB_ASM_LIST}
	${LIB_OBJ_LIST}
)

target_compile_definitions(
	jnc_api_ext
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

target_precompile_headers(
	jnc_api_ext
	PRIVATE
	${PCH_H}
	${PCH_DYNAMIC_CPP}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_api_core
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

install(
	TARGETS jnc_api_ext
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

#...............................................................................
