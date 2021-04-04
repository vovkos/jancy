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

SslStateBase*
SslStateBase::createExternal(
	Runtime* runtime,
	const Guid& libGuid,
	size_t cacheSlotIdx,
	axl::io::Socket* socket
	)
{
	Module* module = runtime->getModule();
	FindModuleItemResult findResult = module->findExtensionLibItem("io.SslState.createSslState", &libGuid, cacheSlotIdx);
	if (!findResult.m_item || findResult.m_item->getItemKind() != ModuleItemKind_Function)
		return err::fail<SslStateBase*>(NULL, "'io.SslState.createSslState' not found");

	Function* function = (Function*)findResult.m_item;
	return callFunction<SslStateBase*>(function, socket);
}

//..............................................................................

} // namespace jnc
} // namespace io
