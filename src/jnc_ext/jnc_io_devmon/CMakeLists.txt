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
# shortcut -- protocol-only io_devmon.jncx
#

if(NOT DEVMON_FOUND)
	set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})

	configure_file(
		${CMAKE_CURRENT_LIST_DIR}/jnc/.forced-proto.jnc
		${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
		COPYONLY
	)

	add_custom_target(
		jnc_io_devmon_jncx
		ALL
		COMMAND ${CMAKE_COMMAND} -E remove ${DLL_DIR}/io_devmon.jncx
		COMMAND
			${7Z_EXE} a -tzip -y
			${DLL_DIR}/io_devmon.jncx
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_DeviceMonitorMi.jnc
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_DeviceMonitorNotify.jnc
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_win_DeviceMonitorNotify.jnc
			${CMAKE_CURRENT_LIST_DIR}/jnc/io_lnx_DeviceMonitorNotify.jnc
			${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
		)

	set_target_properties(
		jnc_io_devmon_jncx
		PROPERTIES
		FOLDER extensions
	)

	install(
		FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_devmon.jncx
		DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
	)

	return()
endif()

#...............................................................................
#
# lib folder
#

set(LIB_DEP_H ${JANCY_BUILD_INC_DIR}/jnc_io_DevMonLibDep.h)

set(
	LIB_H_LIST
	jnc_io_DevMonLib.h
	jnc_io_DeviceMonitor.h
	${LIB_DEP_H}
)

set(
	LIB_CPP_LIST
	jnc_io_DevMonLib.cpp
	jnc_io_DeviceMonitor.cpp
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
	jnc/.forced-proto.jnc
	jnc/io_DeviceMonitor.jnc
	jnc/io_DeviceMonitorMi.jnc
	jnc/io_win_DeviceMonitorNotify.jnc
	jnc/io_lnx_DeviceMonitorNotify.jnc
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

set(PCH_H   pch.h)

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_io_devmon library
#

if(NOT WIN32 AND NOT LINUX)
	message(FATAL "jnc_io_devmon is currently Windows/Linux only")
endif()

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${DEVMON_INC_DIR}
	../jnc_io_cmn
)

link_directories(
	${AXL_LIB_DIR}
	${DEVMON_LIB_DIR}
)

add_library(
	jnc_io_devmon
	SHARED
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
)

target_compile_definitions(
	jnc_io_devmon
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

if(GCC AND NOT APPLE)
	axl_set_export_version_script_gcc(
		jnc_io_devmon
		jncDynamicExtensionLibMain
	)
endif()

target_precompile_headers(
	jnc_io_devmon
	PRIVATE
	${PCH_H}
)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_devmon
	PROPERTIES
	OUTPUT_NAME io_devmon-${TARGET_CPU}
	SUFFIX .bin
)

target_link_libraries(
	jnc_io_devmon
	jnc_io_cmn
	jnc_api_ext
)

if(WIN32)
	target_link_libraries(
		jnc_io_devmon
		dm_win_lib
	)
elseif(LINUX)
	target_link_libraries(
		jnc_io_devmon
		dm_lnx_lib
		pthread
		dl
		rt
	)
endif()

target_link_libraries(
	jnc_io_devmon
	axl_io
	axl_core
)

set_target_properties(
	jnc_io_devmon
	PROPERTIES
	FOLDER extensions
)

#...............................................................................
#
# io_devmon.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_DEVMON_NAME io_devmon-${TARGET_CPU}.bin)
set(JNC_IO_DEVMON_DLL ${DLL_DIR}/${JNC_IO_DEVMON_NAME})
set(JNC_IO_DEVMON_JNCX ${DLL_DIR}/io_devmon.jncx)

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/jnc/.forced-full.jnc
	${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
	COPYONLY
)

add_custom_command(
	TARGET jnc_io_devmon
	POST_BUILD
	COMMAND
		echo Creating io_devmon.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_DEVMON_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_DEVMON_JNCX}
		${JNC_IO_DEVMON_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/io_*.jnc
		${CMAKE_CURRENT_BINARY_DIR}/.forced.jnc
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_devmon.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
