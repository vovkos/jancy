#include "pch.h"
#include "jnc_io_IoLib.h"

//.............................................................................

void
initIoLib (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_ioLibCacheSlot = host->getLibCacheSlot (jnc::io::g_ioLibGuid);
}

AXL_EXPORT
jnc::ext::ExtensionLib* 
getJncExtensionLib (jnc::ext::ExtensionLibHost* host)
{
	static int32_t onceFlag = 0;
	mt::callOnce (initIoLib, host, &onceFlag);
	return mt::getSimpleSingleton <jnc::io::IoLib> ();
}

//.............................................................................
