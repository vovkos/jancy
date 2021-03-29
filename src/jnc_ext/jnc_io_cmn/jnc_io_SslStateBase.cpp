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

#include "pch.h"
#include "jnc_io_SslStateBase.h"
#include "jnc_io_SocketBase.h"

namespace jnc {
namespace io {

//..............................................................................

void
SslStateBase::closeSsl()
{
	m_ssl.close();
	m_sslBio.close();
	m_sslCtx.close();
}

//..............................................................................

} // namespace jnc
} // namespace io
