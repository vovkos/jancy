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

if(NOT ${BUILD_JNC_APP})
	return()
endif()

set (_COMMON_FLAGS -c --ignore-opaque -I ../../api -I ../../common -I ../../protocols -I ../../plugins)

add_jancy_tests(
	NAME_PREFIX "jnc-ioninja-plugin-"
	FLAGS ${_COMMON_FLAGS}

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Echo
	REQUIRES EchoLayer
	FILES EchoLayer.jnc
	FINALIZE_TEST Echo

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/EzTapPro
	REQUIRES
		EzTapProSession
		representSerialLog
	FILES
		EzTapProSession.jnc
		EzTapPro.jnc
		../Serial/SerialLogRecordCode.jnc
		../Serial/SerialLogRepresenter.jnc
	FINALIZE_TEST EzTapPro

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/File
	REQUIRES
		FileSession
		representFileLog
	FILES
		FileSession.jnc
		FileLogRecordCode.jnc
		FileLogRepresenter.jnc
	FINALIZE_TEST File

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/GenericSerialTap
	REQUIRES
		SerialTapSession
		representSerialLog
	FILES
		SerialTapSession.jnc
		../Serial/SerialLogRecordCode.jnc
		../Serial/SerialLogRepresenter.jnc
	FINALIZE_TEST GenericSerialTap

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Mailslot
	REQUIRES
		MailslotSession
		representMailslotLog
	FILES
		MailslotSession.jnc
		MailslotLogRecordCode.jnc
		MailslotLogRepresenter.jnc
	FINALIZE_TEST Mailslot

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Modbus
	REQUIRES
		ModbusLayer
		representModbusLog
	FILES
		ModbusLayer.jnc
		ModbusParserBase.jnc
		ModbusRtuParser.jnc
		ModbusAsciiParser.jnc
		ModbusTcpParser.jnc
		ModbusInfoSet.jnc
		ModbusLogRecordCode.jnc
		ModbusLogRepresenter.jnc
		ModbusLog_Pdu.jnc
		ModbusLog_Rtu.jnc
		ModbusLog_Ascii.jnc
		ModbusLog_Tcp.jnc
	FINALIZE_TEST Modbus

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PipeServer
	REQUIRES
		PipeServerSession
		representPipeServerLog
	FILES
		PipeServerSession.jnc
		PipeClientLogFilter.jnc
		PipeServerLogRecordCode.jnc
		PipeServerLogRepresenter.jnc
	FINALIZE_TEST PipeServer

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/RegexColorizer
	REQUIRES RegexColorizerLayer
	FILES
		RegexPattern.jnc
		RegexColorizerLayer.jnc
		RegexColorizerHighlighter.jnc
	FINALIZE_TEST RegexColorizer

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Serial
	REQUIRES
		SerialSession
		representSerialLog
	FILES
		SerialSession.jnc
		SerialLogRecordCode.jnc
		SerialLogRepresenter.jnc
	FINALIZE_TEST Serial

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SocketLog
	REQUIRES representSocketLog
	FILES
		SocketLogRecordCode.jnc
		SocketLogRepresenter.jnc
	FINALIZE_TEST SocketLog

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpConnection
	REQUIRES
		TcpConnectionSession
		representSocketLog
	FILES
		TcpConnectionSession.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TcpConnection

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpServer
	REQUIRES
		TcpServerSession
		representSocketLog
	FILES
		TcpServerSession.jnc
		../SocketLog/SocketClientLogFilter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TcpServer

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpProxy
	REQUIRES
		TcpProxySession
		representSocketLog
	FILES
		TcpProxySession.jnc
		../SocketLog/SocketClientLogFilter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TcpProxy

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Process
	REQUIRES
		ProcessSession
		representProcessLog
	FILES
		ProcessSession.jnc
		ProcessLogRecordCode.jnc
		ProcessLogRepresenter.jnc
	FINALIZE_TEST Process

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TibboModbusGatewayMon
	REQUIRES
		ModbusGatewayMonSession
		representSocketLog
	FILES
		ModbusGatewayMonSession.jnc
		ModbusGatewayMonLogRecordCode.jnc
		ModbusGatewayMonLogRepresenter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TibboModbusGatewayMon

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TxModifier
	REQUIRES TxModifierLayer
	FILES TxModifierLayer.jnc
	FINALIZE_TEST TxModifier

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TxRxFilter
	REQUIRES TxRxFilterLayer
	FILES TxRxFilterLayer.jnc
	FINALIZE_TEST TxRxFilter

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Udp
	REQUIRES
		UdpSession
		representSocketLog
	FILES
		UdpSession.jnc
		UdpLogRecordCode.jnc
		UdpLogRepresenter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST Udp

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/XModem
	REQUIRES XModemLayer
	FILES
		XModem.jnc
		XModemLayer.jnc
		XModemTransmitter.jnc
		XModemReceiver.jnc
	FINALIZE_TEST XModem

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/NetSnifferLog
	REQUIRES
		representNetSnifferLog
		detailizeNetSnifferLog
	FILES
		NetSnifferLogFilter.jnc
		NetSnifferLogRecordCode.jnc
		NetSnifferLogRepresenter.jnc
		Protocols.jnc
		Proto_Simple.jnc
		Proto_Ethernet.jnc
		Proto_Loopback.jnc
		Proto_Arp.jnc
		Proto_Ip.jnc
		Proto_Ip6.jnc
		Proto_Tcp.jnc
		Proto_Udp.jnc
		Proto_Dns.jnc
		Proto_Dhcp.jnc
	FINALIZE_TEST NetSnifferLog

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpFlow
	REQUIRES
		TcpFlowLayer
		representTcpFlowLog
		representPcapLog
	FILES
		TcpFlowLayer.jnc
		TcpDecoder.jnc
		TcpSequencer.jnc
		TcpFlowLogFilter.jnc
		TcpFlowLogRecordCode.jnc
		TcpFlowLogRepresenter.jnc
		../PcapLog/PcapLogRecordCode.jnc
		../PcapLog/PcapLogRepresenter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TcpFlowAnalyzer

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UdpFlow
	REQUIRES
		UdpFlowLayer
		representUdpFlowLog
		representPcapLog
	FILES
		UdpFlowLayer.jnc
		UdpDecoder.jnc
		UdpFlowLogFilter.jnc
		UdpFlowLogRecordCode.jnc
		UdpFlowLogRepresenter.jnc
		../PcapLog/PcapLogRecordCode.jnc
		../PcapLog/PcapLogRepresenter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST UdpFlowAnalyzer
	)

if(BUILD_JNC_IO_SSH)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS ${_COMMON_FLAGS}

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SshChannel
		REQUIRES
			SshChannelSession
			representSshLog
			representSocketLog
		FILES
			SshChannelSession.jnc
			SshLogRecordCode.jnc
			SshLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST SshChannel

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialMon
		REQUIRES
			SshSerialMonSession
			representSerialMonLog
			representSerialLog
			representSshLog
			representSocketLog
		FILES
			SshSerialMonSession.jnc
			SerialMonitor_mi.jnc
			SerialMonProcessor.jnc
			SerialMonProcessor_win.jnc
			SerialMonProcessor_lnx.jnc
			SerialMonLogRecordCode.jnc
			SerialMonLogRepresenter.jnc
			../Serial/SerialLogRecordCode.jnc
			../Serial/SerialLogRepresenter.jnc
			../SshChannel/SshLogRecordCode.jnc
			../SshChannel/SshLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST SshSerialMon
		)
endif()

if(BUILD_JNC_IO_SSL)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS ${_COMMON_FLAGS}

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SslConnection
		REQUIRES
			SslConnectionSession
			representSslLog
			representSocketLog
		FILES
			SslConnectionSession.jnc
			../SslLog/SslLogRecordCode.jnc
			../SslLog/SslLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST SslConnection

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SslServer
		REQUIRES
			SslServerSession
			representSslLog
			representSocketLog
		FILES
			SslServerSession.jnc
			../SslLog/SslLogRecordCode.jnc
			../SslLog/SslLogRepresenter.jnc
			../SocketLog/SocketClientLogFilter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST SslServer

#		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/WebSocketClient
#		REQUIRES
#			WebSocketClientSession
#			representSslLog
#			representSocketLog
#		FILES
#			WebSocketClientSession.jnc
#			../SslLog/SslLogRecordCode.jnc
#			../SslLog/SslLogRepresenter.jnc
#			../SocketLog/SocketLogRecordCode.jnc
#			../SocketLog/SocketLogRepresenter.jnc
#		FINALIZE_TEST WebSocketClient
#
#		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/WebSocketServer
#		REQUIRES
#			WebSocketServerSession
#			representSslLog
#			representSocketLog
#		FILES
#			WebSocketServerSession.jnc
#			../SslLog/SslLogRecordCode.jnc
#			../SslLog/SslLogRepresenter.jnc
#			../SocketLog/SocketClientLogFilter.jnc
#			../SocketLog/SocketLogRecordCode.jnc
#			../SocketLog/SocketLogRepresenter.jnc
#		FINALIZE_TEST WebSocketServer
		)
endif()

if(BUILD_JNC_IO_PCAP)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS ${_COMMON_FLAGS}

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PcapSniffer
		REQUIRES
			PcapSnifferSession
			representPcapLog
			representNetSnifferLog
			detailizeNetSnifferLog
		FILES
			PcapSnifferSession.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../NetSnifferLog/NetSnifferLogFilter.jnc
			../NetSnifferLog/NetSnifferLogRecordCode.jnc
			../NetSnifferLog/NetSnifferLogRepresenter.jnc
			../NetSnifferLog/Protocols.jnc
			../NetSnifferLog/Proto_Simple.jnc
			../NetSnifferLog/Proto_Ethernet.jnc
			../NetSnifferLog/Proto_Loopback.jnc
			../NetSnifferLog/Proto_Arp.jnc
			../NetSnifferLog/Proto_Ip.jnc
			../NetSnifferLog/Proto_Ip6.jnc
			../NetSnifferLog/Proto_Tcp.jnc
			../NetSnifferLog/Proto_Udp.jnc
			../NetSnifferLog/Proto_Dns.jnc
			../NetSnifferLog/Proto_Dhcp.jnc
		FINALIZE_TEST PcapSniffer

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpFlow
		REQUIRES
			TcpFlowMonSession
			representTcpFlowLog
			representPcapLog
		FILES
			TcpFlowMonSession.jnc
			TcpDecoder.jnc
			TcpSequencer.jnc
			TcpFlowLogFilter.jnc
			TcpFlowLogRecordCode.jnc
			TcpFlowLogRepresenter.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST TcpFlowMon

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UdpFlow
		REQUIRES
			UdpFlowMonSession
			representUdpFlowLog
			representPcapLog
		FILES
			UdpFlowMonSession.jnc
			UdpDecoder.jnc
			UdpFlowLogFilter.jnc
			UdpFlowLogRecordCode.jnc
			UdpFlowLogRepresenter.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST UdpFlowMon
		)
endif()

if(BUILD_JNC_IO_USB)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS ${_COMMON_FLAGS}

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialTap
		REQUIRES
			SerialTapSession
			representSerialLog
		FILES
			SerialTapSession.jnc
			../Serial/SerialLogRecordCode.jnc
			../Serial/SerialLogRepresenter.jnc
		FINALIZE_TEST SerialTap

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/I2cSpiTap
		REQUIRES
			I2cSpiTapSession
			representI2cSpiTapLog
		FILES
			I2cSpiTapSession.jnc
			I2cSpiTapLogRecordCode.jnc
			I2cSpiTapLogRepresenter.jnc
			Decoder.jnc
			I2cDecoder.jnc
			SpiDecoder.jnc
		FINALIZE_TEST I2cSpiTap

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/EthernetTap
		REQUIRES
			EthernetTapSession
			representEthernetTapLog
			detailizeEthernetTapLog
		FILES
			EthernetTapSession.jnc
			EthernetTapLogFilter.jnc
			EthernetTapLogRecordCode.jnc
			EthernetTapLogRepresenter.jnc
			../NetSnifferLog/NetSnifferLogFilter.jnc
			../NetSnifferLog/NetSnifferLogRecordCode.jnc
			../NetSnifferLog/NetSnifferLogRepresenter.jnc
			../NetSnifferLog/Protocols.jnc
			../NetSnifferLog/Proto_Simple.jnc
			../NetSnifferLog/Proto_Ethernet.jnc
			../NetSnifferLog/Proto_Loopback.jnc
			../NetSnifferLog/Proto_Arp.jnc
			../NetSnifferLog/Proto_Ip.jnc
			../NetSnifferLog/Proto_Ip6.jnc
			../NetSnifferLog/Proto_Tcp.jnc
			../NetSnifferLog/Proto_Udp.jnc
			../NetSnifferLog/Proto_Dns.jnc
			../NetSnifferLog/Proto_Dhcp.jnc
		FINALIZE_TEST EthernetTap

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UsbControlEndpoint
		REQUIRES
			UsbControlEndpointSession
			representUsbLog
		FILES
			UsbControlEndpointSession.jnc
			../UsbLog/UsbLogRecordCode.jnc
			../UsbLog/UsbLogRepresenter.jnc
		FINALIZE_TEST UsbControlEndpoint

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UsbDataEndpoint
		REQUIRES
			UsbDataEndpointSession
			representUsbLog
		FILES
			UsbDataEndpointSession.jnc
			../UsbLog/UsbLogRecordCode.jnc
			../UsbLog/UsbLogRepresenter.jnc
		FINALIZE_TEST UsbDataEndpoint

		NAME_PREFIX "jnc-ioninja-sample-"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/samples/CypressUploader
		REQUIRES
			CypressUploaderSession
			representUsbLog
		FILES
			CypressUploaderSession.jnc
			../../plugins/UsbLog/UsbLogRecordCode.jnc
			../../plugins/UsbLog/UsbLogRepresenter.jnc
		FINALIZE_TEST CypressUploader

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/samples/HwSerialMon
		REQUIRES
			HwSerialMonSession
			representHwSerialMonLog
		FILES
			HwSerialMonSession.jnc
			HwSerialMonLogRecordCode.jnc
			HwSerialMonLogRepresenter.jnc
			SerialIface.jnc
			Uart.jnc
			Spi.jnc
			I2c.jnc
			../../plugins/UsbLog/UsbLogRecordCode.jnc
			../../plugins/UsbLog/UsbLogRepresenter.jnc
		FINALIZE_TEST HwSerialMon

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/samples/SpiTap
		REQUIRES
			SpiTapSession
			representSpiTapLog
		FILES
			SpiTapSession.jnc
			SpiTapLogRecordCode.jnc
			SpiTapLogRepresenter.jnc
		FINALIZE_TEST SpiTap
		)
endif()

if(BUILD_JNC_IO_DEVMON AND DEVMON_FOUND)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS ${_COMMON_FLAGS}

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialMon
		REQUIRES
			SerialMonSession
			representDeviceMonLog
			representSerialMonLog
			representSerialLog
		FILES
			SerialMonSession.jnc
			SerialMonitor.jnc
			SerialMonProcessor.jnc
			SerialMonProcessor_win.jnc
			SerialMonProcessor_lnx.jnc
			SerialMonLogRecordCode.jnc
			SerialMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
			../Serial/SerialLogRecordCode.jnc
			../Serial/SerialLogRepresenter.jnc
		FINALIZE_TEST SerialMon

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialMon
		REQUIRES
			SerialMiDumpViewSession
			representSerialMonLog
			representSerialLog
			representFileLog
		FILES
			SerialMiDumpViewSession.jnc
			SerialMonitor_mi.jnc
			SerialMonProcessor.jnc
			SerialMonProcessor_win.jnc
			SerialMonProcessor_lnx.jnc
			SerialMonLogRecordCode.jnc
			SerialMonLogRepresenter.jnc
			../Serial/SerialLogRecordCode.jnc
			../Serial/SerialLogRepresenter.jnc
			../File/FileLogRecordCode.jnc
			../File/FileLogRepresenter.jnc
		FINALIZE_TEST SerialMiDumpViewer

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PipeMon
		REQUIRES
			PipeMonSession
			representDeviceMonLog
			representPipeMonLog
		FILES
			PipeMonSession.jnc
			PipeMonitor.jnc
			PipeMonLogFilter.jnc
			PipeMonLogRecordCode.jnc
			PipeMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
		FINALIZE_TEST PipeMon

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/MailslotMon
		REQUIRES
			MailslotMonSession
			representDeviceMonLog
			representMailslotMonLog
		FILES
			MailslotMonSession.jnc
			MailslotMonitor.jnc
			MailslotMonLogFilter.jnc
			MailslotMonLogRecordCode.jnc
			MailslotMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
		FINALIZE_TEST MailslotMon
		)
endif()

add_jancy_tests(
	NAME_PREFIX "jnc-ioninja-packet-"
	FLAGS -c --ignore-opaque -I ../api -I ../common -I ../protocols
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/packets

	TESTS
		default.jnc
		Modbus.jnc
		TcpIp.jnc
	)

#...............................................................................
