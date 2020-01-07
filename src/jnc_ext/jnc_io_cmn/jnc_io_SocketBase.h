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

#include "jnc_io_SocketAddress.h"
#include "jnc_io_SocketOptions.h"
#include "jnc_io_AsyncIoDevice.h"

namespace jnc {
namespace io {

//..............................................................................

class SocketBase: public AsyncIoDevice
{
protected:
	struct IncomingConnection: sl::ListLink
	{
		axl::io::Socket m_socket;
		axl::io::SockAddr m_sockAddr;
	};

protected:
	axl::io::Socket m_socket;
	uint16_t m_family;

	mem::Pool<IncomingConnection> m_incomingConnectionPool;
	sl::List<IncomingConnection> m_pendingIncomingConnectionList;

protected:
	SocketAddress
	getAddress();

	SocketAddress
	getPeerAddress();

	bool
	setOptions(uint_t options);

	bool
	open(
		uint16_t family,
		int protocol,
		const SocketAddress* address
		);

	void
	close();

	bool
	connectLoop(uint_t connectCompletedEvent);

	void
	acceptLoop(uint_t incomingConnectionEvent);
};

//..............................................................................

} // namespace io
} // namespace jnc
