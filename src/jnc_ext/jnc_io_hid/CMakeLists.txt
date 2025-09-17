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

set(LIB_DEP_H ${JANCY_BUILD_INC_DIR}/jnc_io_HidLibDep.h)

set(
	LIB_H_LIST
	jnc_io_HidLib.h
	jnc_io_HidDevice.h
	jnc_io_HidEnumerator.h
	jnc_io_HidMonEnumerator.h
	jnc_io_HidRd.h
	jnc_io_HidDb.h
	${LIB_DEP_H}
)

set(
	LIB_CPP_LIST
	jnc_io_HidLib.cpp
	jnc_io_HidDevice.cpp
	jnc_io_HidEnumerator.cpp
	jnc_io_HidMonEnumerator.cpp
	jnc_io_HidRd.cpp
	jnc_io_HidDb.cpp
)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

if(NOT USBMON_AVAILABLE)
	axl_exclude_from_build(jnc_io_HidMonEnumerator.cpp)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set(
	JNC_LIST
	jnc/io_HidDevice.jnc
	jnc/io_HidEnumerator.jnc
	jnc/io_HidMonEnumerator.jnc
	jnc/io_HidRd.jnc
	jnc/io_HidDb.jnc
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

set(PCH_H   pch.h)

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_io_hid library
#

include_directories(
	${HIDAPI_INC_DIR}
	${LIBUSB_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
	../jnc_std
)

if(USBMON_AVAILABLE)
	include_directories(${LIBUSB_INC_DIR})
	add_definitions(-D_JNC_IO_HIDMON=1)

	if(${CMAKE_GENERATOR} MATCHES "Visual Studio 10")
		link_directories(${HIDSDK_LIB_DIR})
	endif()
endif()

link_directories(
	${HIDAPI_LIB_DIR}
	${AXL_LIB_DIR}
)

add_library(
	jnc_io_hid
	SHARED
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
)

add_custom_target(
	jnc_io_hid_jnc
	DEPENDS
)

target_compile_definitions(
	jnc_io_hid
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

if(LINUX)
	axl_set_export_version_script_gcc(
		jnc_io_hid
		jncDynamicExtensionLibMain
		jncDynamicExtensionLibUnload
	)
endif()

target_precompile_headers(
	jnc_io_hid
	PRIVATE
	${PCH_H}
)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_hid
	PROPERTIES
	OUTPUT_NAME io_hid-${TARGET_CPU}
	SUFFIX .bin
)

target_link_libraries(
	jnc_io_hid
	jnc_io_cmn
	jnc_api_ext
	jnc_std
	axl_io_hid
	axl_ini
	axl_lex
	axl_zip
	axl_io
	axl_core
	${HIDAPI_LIB_NAME}
)

if(WIN32)
	target_link_libraries(
		jnc_io_hid
		setupapi
	)

	if(USBMON_AVAILABLE)
		target_link_libraries(
			jnc_io_hid
			axl_io_usb
			axl_io_usbmon
			hid
		)
	endif()

	if(HIDAPI_DLL_DIR)
		add_custom_command(
			TARGET jnc_io_hid
			POST_BUILD
			COMMAND
				echo Copying DLL files for jnc_io_hid...
			COMMAND
				${CMAKE_COMMAND} -E copy
				${HIDAPI_DLL_DIR}/hidapi.dll
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
		)
	endif()
else()
	if(APPLE)
		find_library(IOKIT_FRAMEWORK IOKit)
		find_library(APPKIT_FRAMEWORK AppKit)

		target_link_libraries(
			jnc_io_hid
			${IOKIT_FRAMEWORK}
			${APPKIT_FRAMEWORK}
		)
	else()
		if(LINUX)
			target_link_libraries(
				jnc_io_hid
				udev
			)
		endif()

		target_link_libraries(
			jnc_io_hid
			pthread
			dl
			rt
		)
	endif()
endif()

set_target_properties(
	jnc_io_hid
	PROPERTIES
	FOLDER extensions
)

#...............................................................................
#
# io_hid.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_HID_NAME io_hid-${TARGET_CPU}.bin)
set(JNC_IO_HID_DLL  ${DLL_DIR}/${JNC_IO_HID_NAME})
set(JNC_IO_HID_JNCX ${DLL_DIR}/io_hid.jncx)

add_custom_command(
	TARGET jnc_io_hid
	POST_BUILD
	COMMAND
		echo Creating io_hid.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_HID_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_HID_JNCX}
		${JNC_IO_HID_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc
		${AXL_SHARE_DIR}/hid/*.ini
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_hid.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
