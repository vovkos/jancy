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
	jnc/io_PCap.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
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
	)

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
	${JNC_LIST}
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
	OUTPUT_NAME io_pcap-${TARGET_CPU}
	SUFFIX .bin
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
#
# io_pcap.jncx archive
#

set (BIN_DIR ${JANCY_BIN_BASE_DIR}/$(Configuration))

add_custom_target (
	jnc_io_pcap_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${BIN_DIR}/io_pcap.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${BIN_DIR}/io_pcap.jncx ${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc ${BIN_DIR}/io_pcap-${TARGET_CPU}.bin
	DEPENDS
	jnc_io_pcap
	)

#..............................................................................
