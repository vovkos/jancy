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

#if (_AXL_ENV == AXL_ENV_WIN)
#	include "jnc_io_NamedPipe.h"
#endif

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
#if (_AXL_ENV == AXL_ENV_WIN)
		JNC_MAP_TYPE (NamedPipe)
#endif
		JNC_MAP_FUNCTION ("io.createNetworkAdapterDescList", &createNetworkAdapterDescList)
		JNC_MAP_FUNCTION ("io.createSerialPortDescList",     &createSerialPortDescList)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Serial)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Socket)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (SocketAddressResolver)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MappedFile)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (FileStream)
#if (_AXL_ENV == AXL_ENV_WIN)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (NamedPipe)
#endif
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()
};

//.............................................................................

} // namespace io
} // namespace jnc
