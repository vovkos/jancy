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

if (NOT ${BUILD_JNC_APP})
	return ()
endif ()

add_jancy_tests (
	NAME_PREFIX "jnc-ioninja-plugin-"
	FLAGS "-c -I ../../api -I ../../common"

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Echo
	EchoLayer.jnc

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/File
	BEGIN_FILE_LIST
		FileSession.jnc
		FileLog.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/ModBusRtu
	BEGIN_FILE_LIST
		ModBusRtuLayer.jnc
		ModBusRtuLog.jnc
		ModBusLog.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Serial
	BEGIN_FILE_LIST
		SerialSession.jnc
		SerialLog.jnc
		../../common/io_SerialUi.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpConnection
	BEGIN_FILE_LIST
		TcpConnectionSession.jnc
		TcpConnectionLog.jnc
		../../common/io_SocketUi.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpListener
	BEGIN_FILE_LIST
		TcpListenerSession.jnc
		TcpListenerLog.jnc
		../../common/io_SocketUi.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpProxy
	BEGIN_FILE_LIST
		TcpProxySession.jnc
		TcpProxyLog.jnc
		../../common/io_SocketUi.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Udp
	BEGIN_FILE_LIST
		UdpSession.jnc
		UdpLog.jnc
		../../common/io_SocketUi.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/XModem
	BEGIN_FILE_LIST
		XModem.jnc
		XModemLayer.jnc
		XModemReceiver.jnc
		XModemTransmitter.jnc
	END_FILE_LIST
	)

if (BUILD_JNC_IO_SSH)
	add_jancy_tests (
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SshChannel
		BEGIN_FILE_LIST
			SshChannelSession.jnc
			SshChannelLog.jnc
			../../common/io_SocketUi.jnc
		END_FILE_LIST
		)
endif ()

if (BUILD_JNC_IO_PCAP)
	add_jancy_tests (
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Sniffer
		BEGIN_FILE_LIST
			SnifferSession.jnc
			SnifferLog.jnc
			SnifferLog_Ethernet.jnc
			SnifferLog_Ip.jnc
			SnifferLog_Icmp.jnc
			SnifferLog_Tcp.jnc
			SnifferLog_Udp.jnc
			../../common/io_IpDefragmenter.jnc
			../../common/io_PcapDeviceUi.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpFlowMon
		BEGIN_FILE_LIST
			TcpFlowMonSession.jnc
			TcpFlowMonLog.jnc
			../../common/io_IpDefragmenter.jnc
			../../common/io_TcpSequencer.jnc
			../../common/io_PcapDeviceUi.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UdpFlowMon
		BEGIN_FILE_LIST
			UdpFlowMonSession.jnc
			UdpFlowMonLog.jnc
			../../common/io_IpDefragmenter.jnc
			../../common/io_PcapDeviceUi.jnc
		END_FILE_LIST
		)
endif ()

if (BUILD_JNC_IO_USB)
	add_jancy_tests (
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Usb
		BEGIN_FILE_LIST
			UsbSession.jnc
			UsbLog.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/FtdiUsb
		BEGIN_FILE_LIST
			FtdiUsbSession.jnc
			../Usb/UsbLog.jnc
		END_FILE_LIST
		)
endif ()

add_jancy_tests (
	NAME_PREFIX "jnc-ioninja-packet-"
	FLAGS "-c -I ../common"
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/packets

	ModBus.jnc
	PacketPad.jnc
	TcpIp.jnc
	)

#...............................................................................
