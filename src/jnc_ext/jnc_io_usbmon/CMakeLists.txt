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
# shortcut -- transfer-only io_usbmon.jncx
#

if(NOT USBMON_AVAILABLE)
	set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})

	configure_file(
		${CMAKE_CURRENT_LIST_DIR}/jnc/.forced-transfer.jnc
		${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
		COPYONLY
	)

	add_custom_target(
		jnc_io_usbmon_jncx
		ALL
		COMMAND ${CMAKE_COMMAND} -E remove ${DLL_DIR}/io_usbmon.jncx
		COMMAND
			${7Z_EXE} a -tzip -y
			${DLL_DIR}/io_usbmon.jncx
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_UsbMonTransfer.jnc
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_win_UrbFunction.jnc
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_win_UsbdStatus.jnc
			${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
	)

	set_target_properties(
		jnc_io_usbmon_jncx
		PROPERTIES
		FOLDER extensions
	)

	install(
		FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_usbmon.jncx
		DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
	)

	return()
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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
	jnc/.forced-full.jnc
	jnc/.forced-transfer.jnc
	jnc/io_UsbMonEnumerator.jnc
	jnc/io_UsbMonTransfer.jnc
	jnc/io_UsbMonitor.jnc
	jnc/io_win_UsbdStatus.jnc
	jnc/io_win_UrbFunction.jnc
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
	${LIBUSB_INC_DIR}
	../jnc_io_cmn
)

link_directories(${AXL_LIB_DIR})

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
	axl_io_usb
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
		udev
		pthread
		dl
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

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/jnc/.forced-full.jnc
	${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
	COPYONLY
)

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
		${CMAKE_CURRENT_LIST_DIR}/jnc/io_*.jnc
		${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_usbmon.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
