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

set(LIB_DEP_H ${JANCY_BUILD_INC_DIR}/jnc_io_UsbLibDep.h)

set(
	LIB_H_LIST
	jnc_io_UsbDescriptor.h
	jnc_io_UsbEnumerator.h
	jnc_io_UsbEndpoint.h
	jnc_io_UsbInterface.h
	jnc_io_UsbDevice.h
	jnc_io_UsbDeviceFilter.h
	jnc_io_UsbAsyncControlEndpoint.h
	jnc_io_UsbLib.h
	${LIB_DEP_H}
)

set(
	LIB_CPP_LIST
	jnc_io_UsbDescriptor.cpp
	jnc_io_UsbEnumerator.cpp
	jnc_io_UsbEndpoint.cpp
	jnc_io_UsbInterface.cpp
	jnc_io_UsbDevice.cpp
	jnc_io_UsbAsyncControlEndpoint.cpp
	jnc_io_UsbLib.cpp
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
	jnc/io_UsbDescriptors.jnc
	jnc/io_UsbDeviceStrings.jnc
	jnc/io_UsbEnumerator.jnc
	jnc/io_UsbDevice.jnc
	jnc/io_UsbInterface.jnc
	jnc/io_UsbEndpoint.jnc
	jnc/io_UsbTransfer.jnc
)

source_group(
	jnc
	FILES
	${JNC_LIST}
)

touch_if_changed(
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
# jnc_io_usb library
#

include_directories(
	${LIBUSB_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
)

link_directories(
	${LIBUSB_LIB_DIR}
	${AXL_LIB_DIR}
)

add_library(
	jnc_io_usb
	SHARED
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
)

target_compile_definitions(
	jnc_io_usb
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

if(LINUX)
	axl_set_export_version_script_gcc(
		jnc_io_usb
		jncDynamicExtensionLibMain
		jncDynamicExtensionLibUnload
	)
endif()

target_precompile_headers(
	jnc_io_usb
	PRIVATE
	${PCH_H}
)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_usb
	PROPERTIES
	OUTPUT_NAME io_usb-${TARGET_CPU}
	SUFFIX .bin
)

target_link_libraries(
	jnc_io_usb
	jnc_io_cmn
	jnc_api_ext
	axl_io_usb
	axl_io
	axl_core
)

if(WIN32)
	target_link_libraries(
		jnc_io_usb
		setupapi
	)
elseif(UNIX)
	if(APPLE)
		find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
		find_library(IOKIT_FRAMEWORK IOKit)

		target_link_libraries(
			jnc_io_usb
			axl_iok
			axl_cf
			${CORE_FOUNDATION_FRAMEWORK}
			${IOKIT_FRAMEWORK}
		)
	else()
		target_link_libraries(
			jnc_io_usb
			dl
			rt
		)
	endif()
endif()

target_link_libraries(
	jnc_io_usb
	${LIBUSB_LIB_NAME}
)

if(LINUX)
	target_link_libraries(
		jnc_io_usb
		udev
	)
endif()

set_target_properties(
	jnc_io_usb
	PROPERTIES
	FOLDER extensions
)

if(WIN32 AND LIBUSB_DLL_DIR)
	add_custom_command(
		TARGET jnc_io_usb
		POST_BUILD
		COMMAND
			echo Copying DLL files for jnc_io_usb...
		COMMAND
			${CMAKE_COMMAND} -E copy
			${LIBUSB_DLL_DIR}/libusb-1.0.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
		)
endif()

#...............................................................................
#
# io_usb.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_USB_NAME io_usb-${TARGET_CPU}.bin)
set(JNC_IO_USB_DLL ${DLL_DIR}/${JNC_IO_USB_NAME})
set(JNC_IO_USB_JNCX ${DLL_DIR}/io_usb.jncx)

add_custom_command(
	TARGET jnc_io_usb
	POST_BUILD
	COMMAND
		echo Creating io_usb.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_USB_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_USB_JNCX}
		${JNC_IO_USB_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_usb.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
