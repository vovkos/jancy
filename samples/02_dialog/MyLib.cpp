#include "pch.h"
#include "MyLib.h"
#include "MainWindow.h"

//.............................................................................

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
