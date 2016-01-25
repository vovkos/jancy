#pragma once

//.............................................................................

class OutStream
{
public:
	virtual
	size_t
	write (
		const void* p,
		size_t size
		) = 0;

	virtual
	size_t
	printf_va (
		const char* format,
		axl_va_list va
		) = 0;

	size_t
	printf (
		const char* format,
		...
		)
	{
		AXL_VA_DECL (va, format);
		return printf_va (format, va);
	}
};

//.............................................................................

class FileOutStream: public OutStream
{
public:
	FILE* m_file;

public:
	FileOutStream ()
	{
		m_file = stdout;
	}

	virtual
	size_t
	write (
		const void* p,
		size_t size
		)
	{
		ASSERT (m_file);
		return fwrite (p, size, 1, m_file);
	}

	virtual
	size_t
	printf_va (
		const char* format,
		axl_va_list va
		)
	{
		ASSERT (m_file);
		return vfprintf (m_file, format, va.m_va);
	}
};

//.............................................................................

class SocketOutStream: public OutStream
{
public:
	SOCKET m_socket;

public:
	SocketOutStream ()
	{
		m_socket = INVALID_SOCKET;
	}

	virtual
	size_t
	write (
		const void* p,
		size_t size
		)
	{
		ASSERT (m_socket != INVALID_SOCKET);
		return send (m_socket, (const char*) p, size, 0);
	}

	virtual
	size_t
	printf_va (
		const char* format,
		axl_va_list va
		);
};

//.............................................................................
