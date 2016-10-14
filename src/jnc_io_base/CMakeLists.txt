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
	jnc_io_File.h
	jnc_io_MappedFile.h
	jnc_io_FileStream.h
	jnc_io_IoLib.h
	)

set (
	LIB_CPP_LIST
	jnc_io_Serial.cpp
	jnc_io_Socket.cpp
	jnc_io_SocketAddressResolver.cpp
	jnc_io_NetworkAdapter.cpp
	jnc_io_File.cpp
	jnc_io_MappedFile.cpp
	jnc_io_FileStream.cpp
	jnc_io_IoLib.cpp
	)

if (WIN32)
	set (
		LIB_H_LIST
		${LIB_H_LIST}
		jnc_io_NamedPipe.h
		jnc_io_Mailslot.h
		)

	set (
		LIB_CPP_LIST
		${LIB_CPP_LIST}
		jnc_io_NamedPipe.cpp
		jnc_io_Mailslot.cpp
		)
endif ()

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
	jnc/io_Serial.jnc
	jnc/io_Socket.jnc
	jnc/io_SocketAddress.jnc
	jnc/io_SocketAddressResolver.jnc
	jnc/io_NetworkAdapter.jnc
	jnc/io_File.jnc
	jnc/io_MappedFile.jnc
	jnc/io_FileStream.jnc
	)

if (WIN32)
	set (
		JNC_LIST
		${JNC_LIST}
		jnc/io_NamedPipe.jnc
		jnc/io_Mailslot.jnc
		)
endif ()

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
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
	)

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
	${JNC_LIST}
	)

axl_set_pch (
	jnc_io_base
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

target_compile_definitions (
	jnc_io_base
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB
	)

set_target_properties (
	jnc_io_base
	PROPERTIES
	OUTPUT_NAME io_base-${AXL_CPU}
	SUFFIX .bin
	)

target_link_libraries (
	jnc_io_base
	jnc_io_cmn
	jnc_api_dynamic
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

	if (APPLE)
		find_library (CORE_FOUNDATION_FRAMEWORK CoreFoundation)
		find_library (IOKIT_FRAMEWORK IOKit)

		target_link_libraries (
			jnc_io_base
			axl_iok
			axl_cf
			${CORE_FOUNDATION_FRAMEWORK}
			${IOKIT_FRAMEWORK}
			)

	else ()
		target_link_libraries (
			jnc_io_base
			rt
			)
	endif ()

	if (LINUX AND LIBUDEV_FOUND)
		target_link_libraries (
			jnc_io_base
			udev
			)
	endif ()
endif ()

#..............................................................................
#
# io_base.jncx archive
#

set (BIN_DIR ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX})

add_custom_target (
	jnc_io_base_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${BIN_DIR}/io_base.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${BIN_DIR}/io_base.jncx ${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc ${BIN_DIR}/io_base-${AXL_CPU}.bin
	DEPENDS
	jnc_io_base
	)

#..............................................................................
