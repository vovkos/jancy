#include "pch.h"
#include "MyLib.h"
#include "MainWindow.h"

//.............................................................................

MyLib*
getMyLib (jnc::ext::ExtensionLibHost* host)
{
	g_myLibCacheSlot = host->getLibCacheSlot (g_myLibGuid);
	return mt::getSimpleSingleton <MyLib> ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
MyLib::stdPrintf (
	const char* format,
	...
	)
{
	va_list va;
	va_start (va, format);
	return getMainWindow ()->output_va (format, va);
}

//.............................................................................
