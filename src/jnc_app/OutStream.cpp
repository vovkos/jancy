#include "pch.h"
#include "OutStream.h"

//.............................................................................

size_t
FileOutStream::printf_va (
	const char* format,
	axl_va_list va
	)
{
	ASSERT (m_file);

	int length = vfprintf (m_file, format, va.m_va);
	fflush (m_file);
	return length;
}

//.............................................................................

size_t
SocketOutStream::printf_va (
	const char* format,
	axl_va_list va
	)
{
	ASSERT (m_socket != INVALID_SOCKET);

	sl::String string;
	string.format_va (format, va);
	return send (m_socket, string, string.getLength (), 0);
}

//.............................................................................
