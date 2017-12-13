//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

namespace jnc {
namespace io {

//..............................................................................

enum SocketOption
{
	SocketOption_ReuseAddr    = 0x04,
	SocketOption_TcpKeepAlive = 0x08,
	SocketOption_TcpNagle     = 0x10,
	SocketOption_TcpReset     = 0x20,
	SocketOption_UdpBroadcast = 0x40,
	SocketOption_RawHdrIncl   = 0x80,
};

//..............................................................................

} // namespace io
} // namespace jnc
