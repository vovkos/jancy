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
	FLAGS "-c --ignore-opaque -I ../../api -I ../../common"

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Echo
	BEGIN_FILE_LIST
		EchoLayer.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/EzTapPro
	BEGIN_FILE_LIST
		EzTapProSession.jnc
		EzTapPro.jnc
		../Serial/SerialLogRecordCode.jnc
		../Serial/SerialLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/File
	BEGIN_FILE_LIST
		FileSession.jnc
		FileLogRecordCode.jnc
		FileLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/GenericSerialTap
	BEGIN_FILE_LIST
		GenericSerialTapSession.jnc
		../Serial/SerialLogRecordCode.jnc
		../Serial/SerialLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Mailslot
	BEGIN_FILE_LIST
		MailslotSession.jnc
		MailslotLogRecordCode.jnc
		MailslotLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Modbus
	BEGIN_FILE_LIST
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
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PipeListener
	BEGIN_FILE_LIST
		PipeListenerSession.jnc
		PipeListenerLogRecordCode.jnc
		PipeListenerLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/RegexColorizer
	BEGIN_FILE_LIST
		RegexPattern.jnc
		RegexColorizerLayer.jnc
		RegexColorizerHighlighter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Serial
	BEGIN_FILE_LIST
		SerialSession.jnc
		SerialLogRecordCode.jnc
		SerialLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SocketLog
	BEGIN_FILE_LIST
		SocketLogRecordCode.jnc
		SocketLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpConnection
	BEGIN_FILE_LIST
		TcpConnectionSession.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpListener
	BEGIN_FILE_LIST
		TcpListenerSession.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpProxy
	BEGIN_FILE_LIST
		TcpProxySession.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TibboModbusGatewayMon
	BEGIN_FILE_LIST
		ModbusGatewayMonSession.jnc
		ModbusGatewayMonLogRecordCode.jnc
		ModbusGatewayMonLogRepresenter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TxModifier
	BEGIN_FILE_LIST
		TxModifierLayer.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TxRxFilter
	BEGIN_FILE_LIST
		TxRxFilterLayer.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/Udp
	BEGIN_FILE_LIST
		UdpSession.jnc
		UdpLogRecordCode.jnc
		UdpLogRepresenter.jnc
		../SocketLog/SocketLogRecordCode.jnc
		../SocketLog/SocketLogRepresenter.jnc
	END_FILE_LIST

	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/XModem
	BEGIN_FILE_LIST
		XModem.jnc
		XModemLayer.jnc
		XModemTransmitter.jnc
		XModemReceiver.jnc
	END_FILE_LIST
	)

if(BUILD_JNC_IO_SSH)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c --ignore-opaque -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SshChannel
		BEGIN_FILE_LIST
			SshChannelSession.jnc
			SshLogRecordCode.jnc
			SshLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		END_FILE_LIST
		)
endif()

if(BUILD_JNC_IO_PCAP)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c --ignore-opaque -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/NetworkSniffer
		BEGIN_FILE_LIST
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
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/TcpFlowMon
		BEGIN_FILE_LIST
			TcpFlowMonSession.jnc
			TcpFlowMonLogRecordCode.jnc
			TcpFlowMonLogRepresenter.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UdpFlowMon
		BEGIN_FILE_LIST
			UdpFlowMonSession.jnc
			UdpFlowMonLogRecordCode.jnc
			UdpFlowMonLogRepresenter.jnc
			../PcapLog/PcapLogRecordCode.jnc
			../PcapLog/PcapLogRepresenter.jnc
			../SocketLog/SocketLogRecordCode.jnc
			../SocketLog/SocketLogRepresenter.jnc
		END_FILE_LIST
		)
endif()

if(BUILD_JNC_IO_USB)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c --ignore-opaque -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialTap
		BEGIN_FILE_LIST
			SerialTapSession.jnc
			../Serial/SerialLogRecordCode.jnc
			../Serial/SerialLogRepresenter.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/I2cSpiSniffer
		BEGIN_FILE_LIST
			I2cSpiSnifferSession.jnc
			I2cSpiSnifferLogRecordCode.jnc
			I2cSpiSnifferLogRepresenter.jnc
			Protocol.jnc
			Uploader.jnc
			Decoder.jnc
			I2cDecoder.jnc
			SpiDecoder.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UsbControlEndpoint
		BEGIN_FILE_LIST
			UsbControlEndpointSession.jnc
			../UsbLog/UsbLogRecordCode.jnc
			../UsbLog/UsbLogRepresenter.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/UsbDataEndpoint
		BEGIN_FILE_LIST
			UsbDataEndpointSession.jnc
			../UsbLog/UsbLogRecordCode.jnc
			../UsbLog/UsbLogRepresenter.jnc
		END_FILE_LIST

		NAME_PREFIX "jnc-ioninja-sample-"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/samples/CypressUploader
		BEGIN_FILE_LIST
			CypressUploaderSession.jnc
			../../plugins/UsbLog/UsbLogRecordCode.jnc
			../../plugins/UsbLog/UsbLogRepresenter.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/samples/HwSerialMon
		BEGIN_FILE_LIST
			HwSerialMonSession.jnc
			HwSerialMonLogRecordCode.jnc
			HwSerialMonLogRepresenter.jnc
			SerialIface.jnc
			Uart.jnc
			Spi.jnc
			I2c.jnc
			../../plugins/UsbLog/UsbLogRecordCode.jnc
			../../plugins/UsbLog/UsbLogRepresenter.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/samples/SpiTap
		BEGIN_FILE_LIST
			SpiTapSession.jnc
			SpiTapLogRecordCode.jnc
			SpiTapLogRepresenter.jnc
		END_FILE_LIST
		)
endif()

if(BUILD_JNC_IO_DEVMON AND DEVMON_FOUND)
	add_jancy_tests(
		NAME_PREFIX "jnc-ioninja-plugin-"
		FLAGS "-c --ignore-opaque -I ../../api -I ../../common"

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/SerialMon
		BEGIN_FILE_LIST
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
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/PipeMon
		BEGIN_FILE_LIST
			PipeMonSession.jnc
			PipeMonitor.jnc
			PipeMonLogRecordCode.jnc
			PipeMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
		END_FILE_LIST

		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/plugins/MailslotMon
		BEGIN_FILE_LIST
			MailslotMonSession.jnc
			MailslotMonitor.jnc
			MailslotMonLogRecordCode.jnc
			MailslotMonLogRepresenter.jnc
			../DeviceMonLog/DeviceMonLogRecordCode.jnc
			../DeviceMonLog/DeviceMonLogRepresenter.jnc
		END_FILE_LIST
		)
endif()

add_jancy_tests(
	NAME_PREFIX "jnc-ioninja-packet-"
	FLAGS "-c -I ../common"
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/packets

	default.jnc
	ModBus.jnc
	TcpIp.jnc
	)

#...............................................................................
