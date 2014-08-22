#pragma once

//.............................................................................

class COutStream
{
public:
	virtual
	size_t
	Printf_va (
		const char* pFormat,
		axl_va_list va
		) = 0;

	size_t
	Printf (
		const char* pFormat,
		...
		)
	{
		AXL_VA_DECL (va, pFormat);
		return Printf_va (pFormat, va);
	}
};

//.............................................................................

class CFileOutStream: public COutStream
{
public:
	FILE* m_pFile;

public:
	CFileOutStream ()
	{
		m_pFile = stdout;
	}

	virtual
	size_t
	Printf_va (
		const char* pFormat,
		axl_va_list va
		);
};

//.............................................................................

class CSocketOutStream: public COutStream
{
public:
	SOCKET m_Socket;

public:
	CSocketOutStream ()
	{
		m_Socket = INVALID_SOCKET;
	}

	virtual
	size_t
	Printf_va (
		const char* pFormat,
		axl_va_list va
		)
	{
		ASSERT (m_Socket != INVALID_SOCKET);

		rtl::CString String;
		String.Format_va (pFormat, va);
		return send (m_Socket, String, String.GetLength (), 0);
	}
};

//.............................................................................
