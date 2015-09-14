#include "pch.h"
#include "OutStream.h"

//.............................................................................

size_t
FileOutStream::printf_va (
	const char* format,
	axl_va_list va
	)
{
	int length = vfprintf (m_file, format, va.m_va);
	fflush (m_file);
	return length;
}

//.............................................................................
