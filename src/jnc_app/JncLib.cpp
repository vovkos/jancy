#include "pch.h"
#include "JncLib.h"

//.............................................................................

int
JncLib::printf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);
	return vprintf (format, va.m_va);
}

//.............................................................................

