#include "pch.h"
#include "OutStream.h"

//.............................................................................

size_t
CFileOutStream::Printf_va (
	const char* pFormat,
	axl_va_list va
	)
{
	int Length = vfprintf (m_pFile, pFormat, va.m_va);
	fflush (m_pFile);
	return Length;
}

//.............................................................................
