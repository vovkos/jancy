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
	jnc_io_Pcap.h
	jnc_io_PcapLib.h
	)

set (
	LIB_CPP_LIST
	jnc_io_Pcap.cpp
	jnc_io_PcapLib.cpp
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
	jnc/io_Pcap.jnc
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

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_io_pcap library
#

include_directories (
	${PCAP_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	)

link_directories (
	${PCAP_LIB_DIR}
	${AXL_LIB_DIR}
	)

add_library (
	jnc_io_pcap
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	)

axl_set_pch (
	jnc_io_pcap
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

target_compile_definitions (
	jnc_io_pcap
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB
	)

set_target_properties (
	jnc_io_pcap
	PROPERTIES
	OUTPUT_NAME io_pcap-${TARGET_CPU}
	SUFFIX .bin
	)

target_link_libraries (
	jnc_io_pcap
	jnc_api_dynamic
	axl_io
	axl_core
	)

if (WIN32)
	target_link_libraries (
		jnc_io_pcap
		wpcap
		)
elseif (UNIX)
	target_link_libraries (
		jnc_io_pcap
		pthread
		pcap
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_io_pcap
			rt
			)
	endif ()
endif ()

set_target_properties (
	jnc_io_pcap
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
#
# io_pcap.jncx archive
#

set (BIN_DIR ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX})

add_custom_target (
	jnc_io_pcap_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${BIN_DIR}/io_pcap.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${BIN_DIR}/io_pcap.jncx ${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc ${BIN_DIR}/io_pcap-${TARGET_CPU}.bin
	DEPENDS
	jnc_io_pcap
	)

set_target_properties (
	jnc_io_pcap_jncx
	PROPERTIES
	FOLDER extensions
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install (
	FILES ${BIN_DIR}/io_pcap.jncx
	DESTINATION ${JANCY_INSTALL_JNCX_SUBDIR}
	)

#...............................................................................
