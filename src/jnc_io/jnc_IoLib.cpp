#include "pch.h"
#include "jnc_IoLib.h"

//.............................................................................

void
initIoLibSlot (jnc::ExtensionLibSlotDb* slotDb)
{
	jnc::g_ioLibSlot = slotDb->getSlot (jnc::g_ioLibGuid);
}

extern "C"
__declspec (dllexport)
jnc::ExtensionLib* 
getExtensionLib (jnc::ExtensionLibSlotDb* slotDb)
{
	static int32_t onceFlag = 0;
	mt::callOnce (initIoLibSlot, slotDb, &onceFlag);
	return rtl::getSimpleSingleton <jnc::IoLib> ();
}

//.............................................................................
