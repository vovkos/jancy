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
	${JANCY_INC_DIR}/jnc_Def.h
	${JANCY_INC_DIR}/jnc_RuntimeStructs.h
	${JANCY_INC_DIR}/jnc_OpKind.h
	${JANCY_INC_DIR}/jnc_Error.h
	${JANCY_INC_DIR}/jnc_ModuleItem.h
	${JANCY_INC_DIR}/jnc_AttributeBlock.h
	${JANCY_INC_DIR}/jnc_Type.h
	${JANCY_INC_DIR}/jnc_DerivableType.h
	${JANCY_INC_DIR}/jnc_ArrayType.h
	${JANCY_INC_DIR}/jnc_BitFieldType.h
	${JANCY_INC_DIR}/jnc_FunctionType.h
	${JANCY_INC_DIR}/jnc_PropertyType.h
	${JANCY_INC_DIR}/jnc_EnumType.h
	${JANCY_INC_DIR}/jnc_StructType.h
	${JANCY_INC_DIR}/jnc_UnionType.h
	${JANCY_INC_DIR}/jnc_ClassType.h
	${JANCY_INC_DIR}/jnc_Alias.h
	${JANCY_INC_DIR}/jnc_Variable.h
	${JANCY_INC_DIR}/jnc_Function.h
	${JANCY_INC_DIR}/jnc_Property.h
	${JANCY_INC_DIR}/jnc_Namespace.h
	${JANCY_INC_DIR}/jnc_Variant.h
	${JANCY_INC_DIR}/jnc_Promise.h
	${JANCY_INC_DIR}/jnc_Unit.h
	${JANCY_INC_DIR}/jnc_Module.h
	${JANCY_INC_DIR}/jnc_Runtime.h
	${JANCY_INC_DIR}/jnc_GcHeap.h
	${JANCY_INC_DIR}/jnc_ExtensionLib.h
	${JANCY_INC_DIR}/jnc_DynamicExtensionLibHost.h
	${JANCY_INC_DIR}/jnc_CallSite.h
	${JANCY_INC_DIR}/jnc_Construct.h
	${JANCY_INC_DIR}/jnc_StdMap.h
	${JANCY_INC_DIR}/jnc_StdHashTable.h
	${JANCY_INC_DIR}/jnc_StdRbTree.h
	${JANCY_INC_DIR}/jnc_WarningSuppression.h
	)

set(
	LIB_DYNAMIC_CPP_LIST
	jnc_OpKind.cpp
	jnc_Error.cpp
	jnc_ModuleItem.cpp
	jnc_AttributeBlock.cpp
	jnc_Type.cpp
	jnc_DerivableType.cpp
	jnc_ArrayType.cpp
	jnc_BitFieldType.cpp
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
	jnc_Promise.cpp
	jnc_Module.cpp
	jnc_Runtime.cpp
	jnc_GcHeap.cpp
	jnc_StdHashTable.cpp
	jnc_StdRbTree.cpp
	)

set(
	LIB_STATIC_CPP_LIST
	${LIB_DYNAMIC_CPP_LIST}
	jnc_DynamicExtensionLibHost.cpp
	)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_STATIC_CPP_LIST}
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
	${JANCY_INC_DIR}/jnc_Pch.h
	)

source_group(
	pch
	FILES
	${PCH_H_LIST}
	${PCH_STATIC_CPP}
	${PCH_STATIC_O_CPP}
	${PCH_DYNAMIC_CPP}
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

add_library(
	jnc_api_core
	${PCH_H_LIST}
	${PCH_STATIC_CPP}
	${LIB_H_LIST}
	${LIB_STATIC_CPP_LIST}
	)

axl_set_pch(
	jnc_api_core
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

	axl_set_pch(
		jnc_api_core_o
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
	)

target_compile_definitions(
	jnc_api_ext
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
	)

axl_set_pch(
	jnc_api_ext
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
