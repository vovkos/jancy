# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_io_PCap.h
	jnc_io_PCapLib.h
	jnc_io_PCapLibGlobals.h
	)

set (
	LIB_CPP_LIST
	jnc_io_PCap.cpp
	jnc_io_PCapLib.cpp
	)

set (
	LIB_JNC_LIST
	jnc_io_PCap.jnc
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	jnc_io_PCap.jnc.cpp
	jnc_io_PCap.jnc
	${JNC2CPP_PL}
	)

pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/jnc_io_PCap.jnc.cpp
	)

exclude_from_build (${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

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
# jnc_io library
#

include_directories (
	${PCAP_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

if (${JDK_FOUND})
	include_directories (
		${JDK_INC_DIR}
		${JDK_INC_DIR_2}
		)

	add_definitions (-D_JDK_FOUND)
endif ()

link_directories (
	${PCAP_LIB_DIR}
	${AXL_LIB_DIR}
	)

add_library (
	jnc_io_pcap
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

set_pch (
	jnc_io_pcap
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties (
	jnc_io_pcap
	PROPERTIES
	OUTPUT_NAME io_pcap
	SUFFIX .jncx
	)

if (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_target_string_property (
		jnc_io_pcap
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

target_link_libraries (
	jnc_io_pcap
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (WIN32)
	target_link_libraries (
		jnc_io_pcap
		wpcap
		)
elseif (UNIX)
	target_link_libraries (
		jnc_io_pcap
		pthread
		pcap
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_io_pcap
			rt
			)
	endif ()
endif ()

#..............................................................................
