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
#include "jnc_io_WebSocket.h"
#include "jnc_io_WebSockLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	WebSocket,
	"io.WebSocket",
	g_webSockLibGuid,
	WebSockLibCacheSlot_WebSocket,
	WebSocket,
	&WebSocket::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(WebSocket)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<WebSocket>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<WebSocket>)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

WebSocket::WebSocket()
{
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);
}

//..............................................................................

} // namespace io
} // namespace jnc
