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

set(CMAKE_INSTALL_RPATH)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

macro(
	touch_if_changed
	_FILE
	# ...
)
	set(_DEPENDENCY_LIST ${ARGN})

	add_custom_command(
		OUTPUT ${_FILE}
		COMMAND ${CMAKE_COMMAND} -E touch ${_FILE}
		DEPENDS ${_DEPENDENCY_LIST}
	)
endmacro()

#...............................................................................

if(LIBUSB_FOUND AND (WIN32 OR LINUX AND PCAP_FOUND))
	set(USBMON_AVAILABLE TRUE CACHE INTERNAL "usbmon availability") # use cache to make it global
else()
	unset(USBMON_AVAILABLE CACHE)
endif()

add_subdirectory(jnc_rtl_core)
add_subdirectory(jnc_rtl_intro)
add_subdirectory(jnc_std)
add_subdirectory(jnc_sys)
add_subdirectory(jnc_io_cmn)

option(
	BUILD_JNC_IO_BASE
	"Build Jancy basic IO (sockets, serial ports, files) extension library "
	ON
)

if(BUILD_JNC_IO_BASE)
	add_subdirectory(jnc_io_base)
endif()

if(OPENSSL_FOUND)
	option(
		BUILD_JNC_IO_SSL
		"Build Jancy SSL extension library"
		ON
	)

	if(BUILD_JNC_IO_SSL)
		add_subdirectory(jnc_io_ssl)
	endif()

	option(
		BUILD_JNC_IO_WEBSOCKET
		"Build Jancy WebSocket extension library"
		ON
	)

	if(BUILD_JNC_IO_WEBSOCKET)
		add_subdirectory(jnc_io_websocket)
	endif()
endif()

if(LIBSSH2_FOUND AND OPENSSL_FOUND)
	option(
		BUILD_JNC_IO_SSH
		"Build Jancy SSH extension library"
		ON
	)

	if(BUILD_JNC_IO_SSH)
		add_subdirectory(jnc_io_ssh)
	endif()
endif()

if(PCAP_FOUND)
	option(
		BUILD_JNC_IO_PCAP
		"Build Jancy Pcap extension library"
		ON
	)

	if(BUILD_JNC_IO_PCAP)
		add_subdirectory(jnc_io_pcap)
	endif()
endif()

if(LIBUSB_FOUND)
	option(
		BUILD_JNC_IO_USB
		"Build Jancy USB extension library"
		ON
	)

	if(BUILD_JNC_IO_USB)
		add_subdirectory(jnc_io_usb)
	endif()
endif()

# build io_usbmon.jncx even if usbmon is not available (for transfer declarations)

option(
	BUILD_JNC_IO_USBMON
	"Build Jancy USB monitoring extension library"
	ON
)

if(BUILD_JNC_IO_USBMON)
	add_subdirectory(jnc_io_usbmon)
endif()

if(HIDAPI_FOUND)
	option(
		BUILD_JNC_IO_HID
		"Build Jancy HID extension library"
		ON
	)

	if(BUILD_JNC_IO_HID)
		add_subdirectory(jnc_io_hid)
	endif()
endif()

# build io_devmon.jncx even if devmon is not available (for protocol declarations)

option(
	BUILD_JNC_IO_DEVMON
	"Build Jancy device monitoring extension library"
	ON
)

if(BUILD_JNC_IO_DEVMON)
	add_subdirectory(jnc_io_devmon)
endif()

option(
	BUILD_IDE_STDLIB
	"Build jancy stdlib (rtl, std, sys) extension library for IDE"
	ON
)

#...............................................................................
