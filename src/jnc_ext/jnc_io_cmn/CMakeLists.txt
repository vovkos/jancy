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
	jnc_io_UsbDeviceStrings.h
	jnc_io_UsbMonDeviceDescBase.h
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
	jnc_io_UsbDeviceStrings.cpp
	jnc_io_UsbMonDeviceDescBase.cpp
)

if(NOT OPENSSL_FOUND)
	axl_exclude_from_build(jnc_io_SslStateBase.cpp)
	axl_exclude_from_build(jnc_io_SslSocketBase.cpp)
endif()

if(NOT LIBUSB_FOUND)
	axl_exclude_from_build(jnc_io_UsbDeviceStrings.cpp)
endif()

if(NOT USBMON_AVAILABLE)
	axl_exclude_from_build(jnc_io_UsbMonDeviceDescBase.cpp)
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

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_io_cmn library
#

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_rt
)

if(OPENSSL_FOUND)
	include_directories(${OPENSSL_INC_DIR})
	add_definitions(-D_JNC_IO_SSL=1)
endif()

if(LIBUSB_FOUND)
	include_directories(${LIBUSB_INC_DIR})
	add_definitions(-D_JNC_IO_USB=1)
endif()

if(USBMON_AVAILABLE)
	add_definitions(-D_JNC_IO_USBMON=1)
endif()


add_library(
	jnc_io_cmn
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

target_compile_definitions(
	jnc_io_cmn
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

target_precompile_headers(
	jnc_io_cmn
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_io_cmn
	PROPERTIES
	FOLDER extensions
)

#...............................................................................
