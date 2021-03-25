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

set(
	LIB_H_LIST
	jnc_io_WebSocket.h
	jnc_io_WebSockLib.h
	)

set(
	LIB_CPP_LIST
	jnc_io_WebSocket.cpp
	jnc_io_WebSockLib.cpp
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
	jnc/io_WebSocket.jnc
	)

source_group(
	jnc
	FILES
	${JNC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H   pch.h)
set(PCH_CPP pch.cpp)

source_group(
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_io_websock library
#

include_directories(
	${OPENSSL_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
	../jnc_std
	)

link_directories(
	${OPENSSL_LIB_DIR}
	${AXL_LIB_DIR}
	)

add_library(
	jnc_io_websock
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
	)

target_compile_definitions(
	jnc_io_websock
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
	)

if(GCC AND NOT APPLE)
	axl_set_export_version_script_gcc(
		jnc_io_websock
		jncDynamicExtensionLibMain
		)
endif()

axl_set_pch(
	jnc_io_websock
	${PCH_H}
	${PCH_CPP}
	)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_websock
	PROPERTIES
	OUTPUT_NAME io_websock-${TARGET_CPU}
	SUFFIX .bin
	)

target_link_libraries(
	jnc_io_websock
	jnc_io_cmn
	jnc_api_ext
	jnc_std
	axl_cry
	axl_io
	axl_io_ssl
	axl_core
	${OPENSSL_SSL_LIB_NAME}
	${OPENSSL_CRYPTO_LIB_NAME}
	)

if(LINUX)
	target_link_libraries(
		jnc_io_websock
		rt
		)
endif()

set_target_properties(
	jnc_io_websock
	PROPERTIES
	FOLDER extensions
	)

if(WIN32)
	set(_DLL_LIST)

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
			TARGET jnc_io_websock
			POST_BUILD
			COMMAND
				echo Copying DLL files for jnc_io_websock...
			COMMAND
				${CMAKE_COMMAND} -E copy
				${_DLL_LIST}
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
			)
	endif()
endif()

#...............................................................................
#
# io_websock.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_WEBSOCK_NAME io_websock-${TARGET_CPU}.bin)
set(JNC_IO_WEBSOCK_DLL ${DLL_DIR}/${JNC_IO_WEBSOCK_NAME})
set(JNC_IO_WEBSOCK_JNCX ${DLL_DIR}/io_websock.jncx)

add_custom_command(
	TARGET jnc_io_websock
	POST_BUILD
	COMMAND
		echo Creating io_websock.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_WEBSOCK_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_WEBSOCK_JNCX}
		${JNC_IO_WEBSOCK_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_websock.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
	)

#...............................................................................
