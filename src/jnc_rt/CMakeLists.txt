# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_rt_Runtime.h
	jnc_rt_GcHeap.h
	jnc_rt_VariantUtils.h
	)

set (
	LIB_CPP_LIST
	jnc_rt_Runtime.cpp
	jnc_rt_GcHeap.cpp
	jnc_rt_VariantUtils.cpp
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
# jnc_rt library
#

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_INC_PRIVATE_DIR}
	${GEN_DIR}
	)

add_library (
	jnc_rt
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

axl_set_pch (
	jnc_rt
	${PCH_H}
	${PCH_CPP}
	)

if (GCC)
	axl_append_target_string_property (
		jnc_rt
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

#..............................................................................
