# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

add_subdirectory (jnc_core)
add_subdirectory (jnc_std)

option (
	BUILD_JNC_IO_BASE
	"Build jancy base IO (sockets, serial ports, files) extension library "
	ON
	)

if (BUILD_JNC_IO_BASE)
	add_subdirectory (jnc_io_base)
endif ()

option (
	BUILD_JNC_IO_SSH
	"Build jancy SSH IO extension library"
	ON
	)

if (BUILD_JNC_IO_SSH)
	add_subdirectory (jnc_io_ssh)
endif ()

option (
	BUILD_JNC_IO_PCAP
	"Build jancy PCAP IO extension library"
	ON
	)

if (BUILD_JNC_IO_PCAP)
	add_subdirectory (jnc_io_pcap)
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
