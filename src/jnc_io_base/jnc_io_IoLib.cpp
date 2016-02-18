#include "pch.h"
#include "jnc_io_IoLib.h"

//.............................................................................

extern "C"
AXL_EXPORT
jnc::ext::ExtensionLib*
jncExtensionLibMain (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_ioLibCacheSlot = host->getLibCacheSlot (jnc::io::g_ioLibGuid);
	return sl::getSimpleSingleton <jnc::io::IoLib> ();
}

jnc::ext::ExtensionLib*
getExtensionLib ()
{
	return sl::getSimpleSingleton <jnc::io::IoLib> ();
}

//.............................................................................
