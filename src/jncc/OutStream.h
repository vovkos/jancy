#pragma once

//.............................................................................

class OutStream
{
public:
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
	printf_va (
		const char* format,
		axl_va_list va
		);
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
	printf_va (
		const char* format,
		axl_va_list va
		)
	{
		ASSERT (m_socket != INVALID_SOCKET);

		rtl::String string;
		string.format_va (format, va);
		return send (m_socket, string, string.getLength (), 0);
	}
};

//.............................................................................
