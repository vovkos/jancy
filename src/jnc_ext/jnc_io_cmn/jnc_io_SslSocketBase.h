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

#include "jnc_io_SocketBase.h"

namespace jnc {
namespace io {

struct SslStateBase;

//..............................................................................

enum SslSocketEvent
{
	SslSocketEvent_SslHandshakeCompleted = 0x0100,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class SslSocketBase: public SocketBase
{
protected:
	bool
	sslHandshakeLoop(
		SslStateBase* sslState,
		bool isClient
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
