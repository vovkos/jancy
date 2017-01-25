#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

#
# lib folder
#

set (
	LIB_H_LIST
	jnc_io_Ssh.h
	jnc_io_SshLib.h
	)

set (
	LIB_CPP_LIST
	jnc_io_Ssh.cpp
	jnc_io_SshLib.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set (
	JNC_LIST
	jnc/io_Ssh.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
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
# jnc_io_ssh library
#

include_directories (
	${LIBSSH2_INC_DIR}
	${OPENSSL_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
	)

link_directories (
	${LIBSSH2_LIB_DIR}
	${OPENSSL_LIB_DIR}
	${AXL_LIB_DIR}
	)

add_library (
	jnc_io_ssh
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

axl_set_pch (
	jnc_io_ssh
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

target_compile_definitions (
	jnc_io_ssh
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB
	)

set_target_properties (
	jnc_io_ssh
	PROPERTIES
	OUTPUT_NAME io_ssh-${AXL_CPU}
	SUFFIX .bin
	)

target_link_libraries (
	jnc_io_ssh
	jnc_io_cmn
	jnc_api_dynamic
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (WIN32)
	target_link_libraries (
		jnc_io_ssh
		ws2_32
		libssh2
		libeay32MT
		ssleay32MT
		)
elseif (UNIX)
	target_link_libraries (
		jnc_io_ssh
		ssh2
		ssl
		crypto
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_io_ssh
			rt
			)
	endif ()
endif ()

set_target_properties (
	jnc_io_ssh
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
#
# io_ssh.jncx archive
#

set (BIN_DIR ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX})

add_custom_target (
	jnc_io_ssh_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${BIN_DIR}/io_ssh.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${BIN_DIR}/io_ssh.jncx ${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc ${BIN_DIR}/io_ssh-${AXL_CPU}.bin
	DEPENDS
	jnc_io_ssh
	)

set_target_properties (
	jnc_io_ssh_jncx
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
