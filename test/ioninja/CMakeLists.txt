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

add_jancy_tests(
	NAME_PREFIX "jnc-ioninja-plugin-"
	FLAGS -c --ignore-opaque -I ../../api -I ../../common

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
		GenericSerialTapSession.jnc
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
		ModbusLogRecordCode.jnc
		ModbusLogRepresenter.jnc
		ModbusLog_Pdu.jnc
		ModbusLog_Rtu.jnc
		ModbusLog_Ascii.jnc
		ModbusLog_Tcp.jnc
	FINALIZE_TEST Modbus

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PipeListener
	REQUIRES
		PipeListenerSession
		representPipeListenerLog
	FILES
		PipeListenerSession.jnc
		PipeListenerLogRecordCode.jnc
		PipeListenerLogRepresenter.jnc
	FINALIZE_TEST PipeListener

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

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpListener
	REQUIRES
		TcpListenerSession
		representSocketLog
	FILES
		TcpListenerSession.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TcpListener

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpProxy
	REQUIRES
		TcpProxySession
		representSocketLog
	FILES
		TcpProxySession.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	FINALIZE_TEST TcpProxy

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
	)

if(BUILD_JNC_IO_SSH)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS -c --ignore-opaque -I ../../api -I ../../common

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
		)
endif()

if(BUILD_JNC_IO_PCAP)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS -c --ignore-opaque -I ../../api -I ../../common

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/NetworkSniffer
		REQUIRES
			SnifferSession
			representSnifferLog
		FILES
			SnifferSession.jnc
			SnifferLogRecordCode.jnc
			SnifferLogRepresenter.jnc
			SnifferLog_Ethernet.jnc
			SnifferLog_Arp.jnc
			SnifferLog_Ip.jnc
			SnifferLog_Icmp.jnc
			SnifferLog_Tcp.jnc
			SnifferLog_Udp.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
		FINALIZE_TEST NetworkSniffer

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpFlowMon
		REQUIRES
			TcpFlowMonSession
			representTcpFlowMonLog
		FILES
			TcpFlowMonSession.jnc
			TcpFlowMonLogRecordCode.jnc
			TcpFlowMonLogRepresenter.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		FINALIZE_TEST TcpFlowMon

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UdpFlowMon
		REQUIRES
			UdpFlowMonSession
			representUdpFlowMonLog
		FILES
			UdpFlowMonSession.jnc
			UdpFlowMonLogRecordCode.jnc
			UdpFlowMonLogRepresenter.jnc
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
		FLAGS -c --ignore-opaque -I ../../api -I ../../common

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialTap
		REQUIRES
			SerialTapSession
			representSerialLog
		FILES
			SerialTapSession.jnc
			../Serial/SerialLogRecordCode.jnc
			../Serial/SerialLogRepresenter.jnc
		FINALIZE_TEST SerialTap

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/I2cSpiSniffer
		REQUIRES
			I2cSpiSnifferSession
			representI2cSpiSnifferLog
		FILES
			I2cSpiSnifferSession.jnc
			I2cSpiSnifferLogRecordCode.jnc
			I2cSpiSnifferLogRepresenter.jnc
			Protocol.jnc
			Uploader.jnc
			Decoder.jnc
			I2cDecoder.jnc
			SpiDecoder.jnc
		FINALIZE_TEST I2cSpiSniffer

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
		FLAGS -c --ignore-opaque -I ../../api -I ../../common

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialMon
		REQUIRES
			SerialMonSession
			representDeviceMonLog
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

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PipeMon
		REQUIRES
			PipeMonSession
			representDeviceMonLog
		FILES
			PipeMonSession.jnc
			PipeMonitor.jnc
			PipeMonLogRecordCode.jnc
			PipeMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
		FINALIZE_TEST PipeMon

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/MailslotMon
		REQUIRES
			MailslotMonSession
			representDeviceMonLog
		FILES
			MailslotMonSession.jnc
			MailslotMonitor.jnc
			MailslotMonLogRecordCode.jnc
			MailslotMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
		FINALIZE_TEST MailslotMon
		)
endif()

add_jancy_tests(
	NAME_PREFIX "jnc-ioninja-packet-"
	FLAGS -c -I ../common
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/packets

	TESTS
		default.jnc
		ModBus.jnc
		TcpIp.jnc
	)

#...............................................................................
