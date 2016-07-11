# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	${JANCY_INC_DIR}/jnc_RuntimeStructs.h
	${JANCY_INC_DIR}/jnc_OpKind.h
	${JANCY_INC_DIR}/jnc_Error.h
	${JANCY_INC_DIR}/jnc_Type.h
	${JANCY_INC_DIR}/jnc_DerivableType.h
	${JANCY_INC_DIR}/jnc_Function.h
	${JANCY_INC_DIR}/jnc_Property.h
	${JANCY_INC_DIR}/jnc_Namespace.h
	${JANCY_INC_DIR}/jnc_Module.h
	${JANCY_INC_DIR}/jnc_Runtime.h
	${JANCY_INC_DIR}/jnc_ExtensionLib.h
	${JANCY_INC_DIR}/jnc_DynamicExtensionLibHost.h
	${JANCY_INC_DIR}/jnc_OpaqueClassTypeInfo.h
	${JANCY_INC_DIR}/jnc_rt_CallSite.h
	${JANCY_INC_DIR}/jnc_ext_Pch.h
	)

set (
	LIB_CPP_LIST
	jnc_OpKind.cpp
	jnc_Error.cpp
	jnc_Type.cpp
	jnc_DerivableType.cpp
	jnc_Function.cpp
	jnc_Property.cpp
	jnc_Namespace.cpp
	jnc_Module.cpp
	jnc_Runtime.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H           pch.h)
set (PCH_STATIC_CPP  pch_static.cpp)
set (PCH_DYNAMIC_CPP pch_dynamic.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_STATIC_CPP}
	${PCH_DYNAMIC_CPP}
	)

#..............................................................................
#
# jnc_api static/dynamic libraries
#

include_directories (
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_INC_PRIVATE_DIR}
	)

add_library (
	jnc_api_static
	${PCH_H}
	${PCH_STATIC_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

axl_set_pch (
	jnc_api_static
	${PCH_H}
	${PCH_STATIC_CPP}
	)

if (GCC)
	axl_append_target_string_property (
		jnc_api_static
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

add_definitions (-D_JNC_DYNAMIC_EXTENSION_LIB)

add_library (
	jnc_api_dynamic
	${PCH_H}
	${PCH_DYNAMIC_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

axl_set_pch (
	jnc_api_dynamic
	${PCH_H}
	${PCH_DYNAMIC_CPP}
	)

if (GCC)
	axl_append_target_string_property (
		jnc_api_dynamic
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

#..............................................................................
