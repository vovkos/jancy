#include "pch.h"
#include "jnc_io_SshLib.h"

//.............................................................................

void
initSshLib (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_sshLibCacheSlot = host->getLibCacheSlot (jnc::io::g_sshLibGuid);
}

AXL_EXPORT
jnc::ext::ExtensionLib* 
getJncExtensionLib (jnc::ext::ExtensionLibHost* host)
{
	static int32_t onceFlag = 0;
	mt::callOnce (initSshLib, host, &onceFlag);
	return mt::getSimpleSingleton <jnc::io::SshLib> ();
}

//.............................................................................
