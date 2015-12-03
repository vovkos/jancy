# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	)

set (
	LIB_CPP_LIST
	jnc_jni.cpp
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
# jnc_jni library
#

include_directories (
	${JDK_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	)

if (WIN32)
	include_directories (${JDK_INC_DIR}/win32)
endif ()

add_library (
	jnc_jni
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

set_pch (
	jnc_jni
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
