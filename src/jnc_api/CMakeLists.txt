# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
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
	${JANCY_INC_DIR}/jnc_Unit.h
	${JANCY_INC_DIR}/jnc_Module.h
	${JANCY_INC_DIR}/jnc_Runtime.h
	${JANCY_INC_DIR}/jnc_GcHeap.h
	${JANCY_INC_DIR}/jnc_ExtensionLib.h
	${JANCY_INC_DIR}/jnc_CallSite.h
	${JANCY_INC_DIR}/jnc_Construct.h
	jnc_DynamicExtensionLibHost.h
	)

set (
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
	jnc_Module.cpp
	jnc_Runtime.cpp
	jnc_GcHeap.cpp
	)

set (
	LIB_STATIC_CPP_LIST
	${LIB_DYNAMIC_CPP_LIST}
	jnc_DynamicExtensionLibHost.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_STATIC_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H           pch.h)
set (PCH_STATIC_CPP  pch_static.cpp)
set (PCH_DYNAMIC_CPP pch_dynamic.cpp)

set (
	PCH_H_LIST
	${PCH_H}
	${JANCY_INC_DIR}/jnc_Pch.h
	)

source_group (
	pch
	FILES
	${PCH_H_LIST}
	${PCH_STATIC_CPP}
	${PCH_DYNAMIC_CPP}
	)

#..............................................................................
#
# jnc_api static/dynamic libraries
#

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct
	${JANCY_SRC_DIR}/jnc_rt
	)

add_library (
	jnc_api_static
	${PCH_H_LIST}
	${PCH_STATIC_CPP}
	${LIB_H_LIST}
	${LIB_STATIC_CPP_LIST}
	)

axl_set_pch (
	jnc_api_static
	${PCH_H}
	${PCH_STATIC_CPP}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

add_library (
	jnc_api_dynamic
	${PCH_H_LIST}
	${PCH_DYNAMIC_CPP}
	${LIB_H_LIST}
	${LIB_DYNAMIC_CPP_LIST}
	)

target_compile_definitions (
	jnc_api_dynamic
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB
	)

axl_set_pch (
	jnc_api_dynamic
	${PCH_H}
	${PCH_DYNAMIC_CPP}
	)

#..............................................................................
