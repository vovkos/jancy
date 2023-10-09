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

file(
	GLOB
	SAMPLE_JNC_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	*.jnc
)

list(
	REMOVE_ITEM
	SAMPLE_JNC_LIST
	91_WebSocketClient.jnc
	92_WebSocketServer.jnc
)

if(NOT BUILD_JNC_IO_SSL)
	list(
		REMOVE_ITEM
		SAMPLE_JNC_LIST
		90_SslSocket.jnc
	)
endif()

if(NOT BUILD_JNC_IO_HID)
	list(
		REMOVE_ITEM
		SAMPLE_JNC_LIST
		93_HidEnum.jnc
	)
endif()

if(NOT BUILD_JNC_IO_HID OR NOT USBMON_AVAILABLE)
	list(
		REMOVE_ITEM
		SAMPLE_JNC_LIST
		93_HidMonEnum.jnc
	)
endif()

source_group(
	jnc
	FILES
	${SAMPLE_JNC_LIST}
)

if(${BUILD_JNC_APP})
	add_jancy_tests(
		NAME_PREFIX "jnc-sample-"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		REQUIRES "main"
		TESTS ${SAMPLE_JNC_LIST}
	)
endif()

#...............................................................................
