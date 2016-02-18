# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (jnc_core)
add_subdirectory (jnc_std)
add_subdirectory (jnc_sys)
add_subdirectory (jnc_io_cmn)

option (
	BUILD_JNC_IO_BASE
	"Build jancy base IO (sockets, serial ports, files) extension library "
	ON
	)

if (BUILD_JNC_IO_BASE)
	add_subdirectory (jnc_io_base)
endif ()

if (${LIBSSH2_FOUND})
	option (
		BUILD_JNC_IO_SSH
		"Build jancy SSH IO extension library"
		ON
		)

	if (BUILD_JNC_IO_SSH)
		add_subdirectory (jnc_io_ssh)
	endif ()
endif ()

if (${PCAP_FOUND})
	option (
		BUILD_JNC_IO_PCAP
		"Build jancy PCAP IO extension library"
		ON
		)

	if (BUILD_JNC_IO_PCAP)
		add_subdirectory (jnc_io_pcap)
	endif ()
endif ()

option (
	BUILD_JNC_APP
	"Build jancy command line application"
	ON
	)

if (BUILD_JNC_APP)
	add_subdirectory (jnc_app)
endif ()

#..............................................................................
