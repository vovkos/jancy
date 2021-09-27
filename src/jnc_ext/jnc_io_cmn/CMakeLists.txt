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
	jnc_io_AsyncIoBase.h
	jnc_io_AsyncIoDevice.h
	jnc_io_SocketAddress.h
	jnc_io_SocketCapabilities.h
	jnc_io_SocketBase.h
	jnc_io_SslStateBase.h
	jnc_io_SslSocketBase.h
)

set(
	LIB_CPP_LIST
	jnc_io_AsyncIoBase.cpp
	jnc_io_AsyncIoDevice.cpp
	jnc_io_SocketAddress.cpp
	jnc_io_SocketCapabilities.cpp
	jnc_io_SocketBase.cpp
	jnc_io_SslStateBase.cpp
	jnc_io_SslSocketBase.cpp
)

if(NOT OPENSSL_FOUND)
	axl_exclude_from_build(jnc_io_SslStateBase.cpp)
	axl_exclude_from_build(jnc_io_SslSocketBase.cpp)
endif()

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H     pch.h)
set(PCH_CPP   pch.cpp)

source_group(
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
)

#...............................................................................
#
# jnc_io_cmn library
#

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
)

if(OPENSSL_FOUND)
	include_directories(${OPENSSL_INC_DIR})
	add_definitions(-D_JNC_IO_SSL=1)
endif()

add_library(
	jnc_io_cmn
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

target_compile_definitions(
	jnc_io_cmn
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

axl_set_pch(
	jnc_io_cmn
	${PCH_H}
	${PCH_CPP}
)

set_target_properties(
	jnc_io_cmn
	PROPERTIES
	FOLDER extensions
)

#...............................................................................
