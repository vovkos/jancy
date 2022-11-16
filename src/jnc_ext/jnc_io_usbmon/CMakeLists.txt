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
	jnc_io_UsbMonLib.h
	jnc_io_UsbMonitor.h
	jnc_io_UsbMonEnumerator.h
)

set(
	LIB_CPP_LIST
	jnc_io_UsbMonLib.cpp
	jnc_io_UsbMonitor.cpp
	jnc_io_UsbMonEnumerator.cpp
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
	jnc/.forced.jnc
	jnc/io_UsbMonEnumerator.jnc
	jnc/io_UsbMonTransfer.jnc
	jnc/io_UsbMonitor.jnc
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
# jnc_io_usbmon library
#

if(NOT WIN32 AND NOT LINUX)
	message(FATAL "jnc_io_usbmon is currently Windows/Linux only")
endif()

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
)

link_directories(${AXL_LIB_DIR})

if(LINUX)
	include_directories(${LIBUSB_INC_DIR})
	link_directories(${LIBUSB_LIB_DIR})
endif()

add_library(
	jnc_io_usbmon
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
)

target_compile_definitions(
	jnc_io_usbmon
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

if(LINUX)
	axl_set_export_version_script_gcc(
		jnc_io_usbmon
		jncDynamicExtensionLibMain
		jncDynamicExtensionLibUnload
	)
endif()

axl_set_pch(
	jnc_io_usbmon
	${PCH_H}
	${PCH_CPP}
)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_usbmon
	PROPERTIES
	OUTPUT_NAME io_usbmon-${TARGET_CPU}
	SUFFIX .bin
)

if(LINUX)
	set(AXL_IO_USB_LIB_NAME axl_io_usb)
else()
	set(AXL_IO_USB_LIB_NAME)
endif()

target_link_libraries(
	jnc_io_usbmon
	jnc_io_cmn
	jnc_api_ext
	axl_io_usbmon
	${AXL_IO_USB_LIB_NAME}
	axl_io
	axl_core
)

if(WIN32)
	target_link_libraries(
		jnc_io_usbmon
		setupapi
	)
elseif(LINUX)
	target_link_libraries(
		jnc_io_usbmon
		${LIBUSB_LIB_NAME}
		pthread
		rt
	)
endif()

set_target_properties(
	jnc_io_usbmon
	PROPERTIES
	FOLDER extensions
)

#...............................................................................
#
# io_usbmon.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_USBMON_NAME io_usbmon-${TARGET_CPU}.bin)
set(JNC_IO_USBMON_DLL ${DLL_DIR}/${JNC_IO_USBMON_NAME})
set(JNC_IO_USBMON_JNCX ${DLL_DIR}/io_usbmon.jncx)

add_custom_command(
	TARGET jnc_io_usbmon
	POST_BUILD
	COMMAND
		echo Creating io_usbmon.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_USBMON_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_USBMON_JNCX}
		${JNC_IO_USBMON_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_usbmon.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
