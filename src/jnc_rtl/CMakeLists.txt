# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#...............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_rtl_Multicast.h
	jnc_rtl_DynamicLib.h
	jnc_rtl_Recognizer.h
	)

set (
	LIB_CPP_LIST
	jnc_rtl_CoreLib.cpp
	jnc_rtl_Multicast.cpp
	jnc_rtl_DynamicLib.cpp
	jnc_rtl_Recognizer.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
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
# jnc_rtl library
#

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct
	${JANCY_SRC_DIR}/jnc_rt
	${GEN_DIR}
	)

add_library (
	jnc_rtl
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

axl_set_pch (
	jnc_rtl
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
