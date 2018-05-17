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
	jnc_io_DevMonLib.h
	jnc_io_DeviceMonitor.h
	)

set (
	LIB_CPP_LIST
	jnc_io_DevMonLib.cpp
	jnc_io_DeviceMonitor.cpp
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
	jnc/.forced.jnc
	jnc/io_DeviceMonitor.jnc
	jnc/io_win_DeviceMonitorNotify.jnc
	jnc/io_lnx_DeviceMonitorNotify.jnc
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

set (PCH_H   pch.h)
set (PCH_CPP pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_io_devmon library
#

if (NOT WIN32 AND NOT LINUX)
	message (FATAL "jnc_io_devmon is currently Windows/Linux only")
endif ()

include_directories (
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${DEVMON_INC_DIR}
	../jnc_io_cmn
	)

link_directories (
	${AXL_LIB_DIR}
	${DEVMON_LIB_DIR}
	)

add_library (
	jnc_io_devmon
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	)

target_compile_definitions (
	jnc_io_devmon
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB
	)

if (GCC AND NOT APPLE)
	axl_set_export_version_script_gcc (
		jnc_io_devmon
		jncDynamicExtensionLibMain
		)
endif ()

axl_set_pch (
	jnc_io_devmon
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties (
	jnc_io_devmon
	PROPERTIES
	OUTPUT_NAME io_devmon-${TARGET_CPU}
	SUFFIX .bin
	)

target_link_libraries (
	jnc_io_devmon
	jnc_io_cmn
	jnc_api_ext
	)

if (WIN32)
	target_link_libraries (
		jnc_io_devmon
		dm_win_lib
		)
elseif (LINUX)
	target_link_libraries (
		jnc_io_devmon
		dm_lnx_lib
		pthread
		rt
		)
endif ()

target_link_libraries (
	jnc_io_devmon
	axl_io
	axl_core
	)

set_target_properties (
	jnc_io_devmon
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
#
# io_devmon.jncx archive
#

set (BIN_DIR ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX})

add_custom_target (
	jnc_io_devmon_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${BIN_DIR}/io_devmon.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${BIN_DIR}/io_devmon.jncx ${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc ${BIN_DIR}/io_devmon-${TARGET_CPU}.bin
	DEPENDS
	jnc_io_devmon
	)

set_target_properties (
	jnc_io_devmon_jncx
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
