#include "pch.h"
#include "jnc_io_PCapLib.h"

//.............................................................................

void
initPCapLib (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_pcapLibCacheSlot = host->getLibCacheSlot (jnc::io::g_pcapLibGuid);
}

AXL_EXPORT
jnc::ext::ExtensionLib* 
getJncExtensionLib (jnc::ext::ExtensionLibHost* host)
{
	static int32_t onceFlag = 0;
	mt::callOnce (initPCapLib, host, &onceFlag);
	return mt::getSimpleSingleton <jnc::io::PCapLib> ();
}

//.............................................................................
