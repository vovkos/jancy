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
	enum IoThreadFlag
	{
		IoThreadFlag_Tcp = 0x0010,
		IoThreadFlag_Ip6 = 0x0020,
	};

protected:
	axl::io::Socket m_socket;

protected:
	SocketAddress
	getAddress ();

	SocketAddress
	getPeerAddress ();

	bool
	setOptions (uint_t options);

	bool
	open (
		uint16_t family,
		int protocol,
		const SocketAddress* address
		);

	void
	close ();

	bool
	tcpConnect (uint_t connectCompletedEvent);
};

//..............................................................................

} // namespace io
} // namespace jnc
