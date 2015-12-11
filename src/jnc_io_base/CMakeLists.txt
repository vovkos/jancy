# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_io_Serial.h
	jnc_io_Socket.h
	jnc_io_SocketAddressResolver.h
	jnc_io_NetworkAdapter.h
	jnc_io_MappedFile.h
	jnc_io_FileStream.h
	jnc_io_IoLib.h
	jnc_io_IoLibGlobals.h
	)

set (
	LIB_CPP_LIST
	jnc_io_Serial.cpp
	jnc_io_Socket.cpp
	jnc_io_SocketAddressResolver.cpp
	jnc_io_NetworkAdapter.cpp
	jnc_io_MappedFile.cpp
	jnc_io_FileStream.cpp
	jnc_io_IoLib.cpp
	)

set (
	LIB_JNC_LIST
	jnc_io_Serial.jnc
	jnc_io_Socket.jnc
	jnc_io_SocketAddress.jnc
	jnc_io_SocketAddressResolver.jnc
	jnc_io_NetworkAdapter.jnc
	jnc_io_MappedFile.jnc
	jnc_io_FileStream.jnc
	)

if (WIN32)
	set (
		LIB_H_LIST
		${LIB_H_LIST}
		jnc_io_NamedPipe.h
		)

	set (
		LIB_CPP_LIST
		${LIB_CPP_LIST}
		jnc_io_NamedPipe.cpp
		)

	set (
		LIB_JNC_LIST
		${LIB_JNC_LIST}
		jnc_io_NamedPipe.jnc
		)
endif ()

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
	jnc_io_Serial.jnc.cpp
	jnc_io_Serial.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_io_Socket.jnc.cpp
	jnc_io_Socket.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_io_SocketAddress.jnc.cpp
	jnc_io_SocketAddress.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_io_SocketAddressResolver.jnc.cpp
	jnc_io_SocketAddressResolver.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_io_NetworkAdapter.jnc.cpp
	jnc_io_NetworkAdapter.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_io_MappedFile.jnc.cpp
	jnc_io_MappedFile.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_io_FileStream.jnc.cpp
	jnc_io_FileStream.jnc
	${JNC2CPP_PL}
	)

if (WIN32)
	add_perl_step (
		jnc_io_NamedPipe.jnc.cpp
		jnc_io_NamedPipe.jnc
		${JNC2CPP_PL}
		)
endif ()

pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/jnc_io_Serial.jnc.cpp
	${GEN_DIR}/jnc_io_Socket.jnc.cpp
	${GEN_DIR}/jnc_io_SocketAddress.jnc.cpp
	${GEN_DIR}/jnc_io_NetworkAdapter.jnc.cpp
	${GEN_DIR}/jnc_io_MappedFile.jnc.cpp
	${GEN_DIR}/jnc_io_FileStream.jnc.cpp
	)

if (WIN32)
	set (
		GEN_JNC_CPP_LIST
		${GEN_JNC_CPP_LIST}
		${GEN_DIR}/jnc_io_NamedPipe.jnc.cpp
		)
endif ()

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
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	../jnc_io_cmn
	)

if (${JDK_FOUND})
	include_directories (
		${JDK_INC_DIR}
		${JDK_INC_DIR_2}
		)

	add_definitions (-D_JDK_FOUND)
endif ()

link_directories (
	${AXL_LIB_DIR}
	)

add_library (
	jnc_io_base
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

set_pch (
	jnc_io_base
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties (
	jnc_io_base
	PROPERTIES
	OUTPUT_NAME io_base
	SUFFIX .jncx
	)

if (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_target_string_property (
		jnc_io_base
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

target_link_libraries (
	jnc_io_base
	jnc_io_cmn
	axl_io
	axl_core
	)

if (WIN32)
	target_link_libraries (
		jnc_io_base
		ws2_32
		)
elseif (UNIX)
	target_link_libraries (
		jnc_io_base		
		pthread
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_io_base
			rt
			)
	endif ()
endif ()

#..............................................................................
