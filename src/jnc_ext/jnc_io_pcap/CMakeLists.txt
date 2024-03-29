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

set(LIB_DEP_H ${JANCY_BUILD_INC_DIR}/jnc_io_PcapLibDep.h)

set(
	LIB_H_LIST
	jnc_io_Pcap.h
	jnc_io_PcapFilter.h
	jnc_io_PcapSignalMgr.h
	jnc_io_PcapLib.h
	${LIB_DEP_H}
)

set(
	LIB_CPP_LIST
	jnc_io_Pcap.cpp
	jnc_io_PcapFilter.cpp
	jnc_io_PcapSignalMgr.cpp
	jnc_io_PcapLib.cpp
)

if(NOT UNIX)
	axl_exclude_from_build(jnc_io_PcapSignalMgr.cpp)
endif()

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
	jnc/io_Pcap.jnc
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
# jnc_io_pcap library
#

include_directories(
	${PCAP_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	../jnc_io_cmn
)

link_directories(
	${PCAP_LIB_DIR}
	${AXL_LIB_DIR}
)

add_library(
	jnc_io_pcap
	SHARED
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
)

if(GCC AND NOT APPLE)
	axl_set_export_version_script_gcc(
		jnc_io_pcap
		jncDynamicExtensionLibMain
	)
endif()

target_compile_definitions(
	jnc_io_pcap
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
)

target_precompile_headers(
	jnc_io_pcap
	PRIVATE
	${PCH_H}
)

set(CMAKE_SHARED_LIBRARY_PREFIX)

set_target_properties(
	jnc_io_pcap
	PROPERTIES
	OUTPUT_NAME io_pcap-${TARGET_CPU}
	SUFFIX .bin
)

target_link_libraries(
	jnc_io_pcap
	jnc_io_cmn
	jnc_api_ext
	axl_io_pcap
	axl_io
	axl_core
)

if(WIN32)
	target_link_libraries(
		jnc_io_pcap
		wpcap
	)

	set_target_properties(
		jnc_io_pcap
		PROPERTIES
		LINK_FLAGS "/DELAYLOAD:wpcap.dll"
	)
elseif(UNIX)
	target_link_libraries(
		jnc_io_pcap
		pthread
		pcap
	)

	if(LINUX)
		target_link_libraries(
			jnc_io_pcap
			dl
			rt
		)
	endif()
endif()

set_target_properties(
	jnc_io_pcap
	PROPERTIES
	FOLDER extensions
)

#...............................................................................
#
# io_pcap.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})
set(JNC_IO_PCAP_NAME io_pcap-${TARGET_CPU}.bin)
set(JNC_IO_PCAP_DLL ${DLL_DIR}/${JNC_IO_PCAP_NAME})
set(JNC_IO_PCAP_JNCX ${DLL_DIR}/io_pcap.jncx)

add_custom_command(
	TARGET jnc_io_pcap
	POST_BUILD
	COMMAND
		echo Creating io_pcap.jncx...
	COMMAND
		${CMAKE_COMMAND} -E remove ${JNC_IO_PCAP_JNCX}
	COMMAND
		${7Z_EXE}
		a -tzip -y
		${JNC_IO_PCAP_JNCX}
		${JNC_IO_PCAP_DLL}
		${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	FILES ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>/io_pcap.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
)

#...............................................................................
