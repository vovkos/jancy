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

set(LIB_DEP_H ${JANCY_BUILD_INC_DIR}/jnc_io_SshLibDep.h)

set(
	LIB_H_LIST
	jnc_io_Ssh.h
	jnc_io_SshLib.h
	${LIB_DEP_H}
)

set(
	LIB_CPP_LIST
	jnc_io_Ssh.cpp
	jnc_io_SshLib.cpp
)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set(
	JNC_LIST
	jnc/io_Ssh.jnc
)

source_group(
	jnc
	FILES
	${JNC_LIST}
)

axl_touch_if_changed(
	${LIB_DEP_H}
	${JNC_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H     pch.h)

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_io_ssh library
#

include_directories(
	${LIBSSH2_INC_DIR}
	${OPENSSL_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
)

link_directories(
	${LIBSSH2_LIB_DIR}
	${OPENSSL_LIB_DIR}
	${AXL_LIB_DIR}
)

add_library(
	jnc_io_ssh
	SHARED
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
)

target_compile_definitions(
	jnc_io_ssh
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

if(GCC AND NOT APPLE)
	axl_set_export_version_script_gcc(
		jnc_io_ssh
		jncDynamicExtensionLibMain
	)
endif()

target_precompile_headers(
	jnc_io_ssh
	PRIVATE
	${PCH_H}
)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_ssh
	PROPERTIES
	OUTPUT_NAME io_ssh-${TARGET_CPU}
	SUFFIX .bin
)

target_link_libraries(
	jnc_io_ssh
	jnc_io_cmn
	jnc_api_ext
	axl_io
	axl_core
	${OPENSSL_SSL_LIB_NAME}
	${OPENSSL_CRYPTO_LIB_NAME}
)

if(WIN32)
	target_link_libraries(
		jnc_io_ssh
		libssh2
	)
else()
	target_link_libraries(
		jnc_io_ssh
		ssh2
	)

	if(NOT APPLE)
		target_link_libraries(
			jnc_io_ssh
			crypto
			pthread
			dl
			rt
		)
	endif()
endif()

set_target_properties(
	jnc_io_ssh
	PROPERTIES
	FOLDER extensions
)

if(WIN32)
	set(_DLL_LIST)

	if(LIBSSH2_DLL_DIR)
		list(APPEND _DLL_LIST ${LIBSSH2_DLL_DIR}/libssh2.dll)
	endif()

	if(OPENSSL_DLL_DIR)
		list(
			APPEND
			_DLL_LIST
			${OPENSSL_DLL_DIR}/${OPENSSL_SSL_DLL_NAME}.dll
			${OPENSSL_DLL_DIR}/${OPENSSL_CRYPTO_DLL_NAME}.dll
		)
	endif()

	if(_DLL_LIST)
		add_custom_command(
			TARGET jnc_io_ssh
			POST_BUILD
			COMMAND
				echo Copying DLL files for jnc_io_ssh...
			COMMAND
				${CMAKE_COMMAND} -E copy
				${_DLL_LIST}
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
		)
	endif()
endif()

#...............................................................................
#
# io_ssh.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_SSH_NAME io_ssh-${TARGET_CPU}.bin)
set(JNC_IO_SSH_DLL ${DLL_DIR}/${JNC_IO_SSH_NAME})
set(JNC_IO_SSH_JNCX ${DLL_DIR}/io_ssh.jncx)

add_custom_command(
	TARGET jnc_io_ssh
	POST_BUILD
	COMMAND
		echo Creating io_ssh.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_SSH_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_SSH_JNCX}
		${JNC_IO_SSH_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_ssh.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
