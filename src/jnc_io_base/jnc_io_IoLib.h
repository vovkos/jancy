// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_io_Serial.h"
#include "jnc_io_Socket.h"
#include "jnc_io_SocketAddress.h"
#include "jnc_io_SocketAddressResolver.h"
#include "jnc_io_NetworkAdapter.h"
#include "jnc_io_MappedFile.h"
#include "jnc_io_FileStream.h"
#include "jnc_io_NamedPipe.h"

#include "jnc_io_Serial.jnc.cpp"
#include "jnc_io_Socket.jnc.cpp"
#include "jnc_io_SocketAddress.jnc.cpp"
#include "jnc_io_SocketAddressResolver.jnc.cpp"
#include "jnc_io_NetworkAdapter.jnc.cpp"
#include "jnc_io_MappedFile.jnc.cpp"
#include "jnc_io_FileStream.jnc.cpp"
#include "jnc_io_NamedPipe.jnc.cpp"

namespace jnc {
namespace io {

//.............................................................................

class IoLib: public ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_TYPE (Serial)
		JNC_MAP_TYPE (Socket)
		JNC_MAP_TYPE (Address_ip4)
		JNC_MAP_TYPE (Address_ip6)
		JNC_MAP_TYPE (SocketAddress_ip4)
		JNC_MAP_TYPE (SocketAddress_ip6)
		JNC_MAP_TYPE (SocketAddress)
		JNC_MAP_TYPE (SocketAddressResolver)
		JNC_MAP_TYPE (MappedFile)
		JNC_MAP_TYPE (FileStream)
		JNC_MAP_TYPE (NamedPipe)

		JNC_MAP_FUNCTION ("io.createNetworkAdapterDescList", &createNetworkAdapterDescList)
		JNC_MAP_FUNCTION ("io.createSerialPortDescList",     &createSerialPortDescList)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Serial)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Socket)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (SocketAddressResolver)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MappedFile)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (FileStream)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (NamedPipe)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

	JNC_BEGIN_LIB_SOURCE_FILE_TABLE ()
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_Serial.jnc", g_io_SerialSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_Socket.jnc", g_io_SocketSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_SocketAddress.jnc", g_io_SocketAddressSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_SocketAddressResolver.jnc", g_io_SocketAddressResolverSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_NetworkAdapter.jnc", g_io_NetworkAdapterSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_MappedFile.jnc", g_io_MappedFileSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_FileStream.jnc", g_io_FileStreamSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_NamedPipe.jnc", g_io_NamedPipeSrc)
	JNC_END_LIB_SOURCE_FILE_TABLE ()
};

//.............................................................................

} // namespace io
} // namespace jnc
