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

class SocketBase;

//..............................................................................

struct SslStateBase: public IfaceHdr {
	ClassBox<Multicast> m_onStateChanged;

	axl::io::SslCtx m_sslCtx;
	axl::cry::Bio m_sslBio;
	axl::io::Ssl m_ssl;

	void
	closeSsl();

	static
	SslStateBase*
	createExternal(
		Runtime* runtime,
		const Guid& libGuid,
		size_t cacheSlotIdx,
		axl::io::Socket* socket
	);
};

//..............................................................................

} // namespace io
} // namespace jnc
