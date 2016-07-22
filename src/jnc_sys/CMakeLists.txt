# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_sys_Event.h
	jnc_sys_Lock.h
	jnc_sys_Thread.h
	jnc_sys_Timer.h
	jnc_sys_SysLib.h
	)

set (
	LIB_CPP_LIST
	jnc_sys_Event.cpp
	jnc_sys_Lock.cpp
	jnc_sys_Thread.cpp
	jnc_sys_Timer.cpp
	jnc_sys_SysLib.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set (
	JNC_LIST
	jnc/sys_globals.jnc
	jnc/sys_Event.jnc
	jnc/sys_Lock.jnc
	jnc/sys_Thread.jnc
	jnc/sys_Timer.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	sys_globals.jnc.cpp
	jnc/sys_globals.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	sys_Event.jnc.cpp
	jnc/sys_Event.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	sys_Lock.jnc.cpp
	jnc/sys_Lock.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	sys_Thread.jnc.cpp
	jnc/sys_Thread.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	sys_Timer.jnc.cpp
	jnc/sys_Timer.jnc
	${JNC2CPP_PL}
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/sys_globals.jnc.cpp
	${GEN_DIR}/sys_Event.jnc.cpp
	${GEN_DIR}/sys_Lock.jnc.cpp
	${GEN_DIR}/sys_Thread.jnc.cpp
	${GEN_DIR}/sys_Timer.jnc.cpp
	)

axl_exclude_from_build (${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

source_group (
	gen
	FILES
	${GEN_JNC_CPP_LIST}
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
# jnc_std library
#

include_directories (
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

add_library (
	jnc_sys
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

axl_set_pch (
	jnc_sys
	${PCH_H}
	${PCH_CPP}
	)

if (GCC)
	axl_append_target_string_property (
		jnc_sys
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

#..............................................................................
