#include "pch.h"
#include "jncc.h"
#include "jncc_OutStream.h"

//.............................................................................

int
CJnc::Server ()
{
	int Result;

	PrintVersion (m_pOutStream);

#if (_AXL_ENV == AXL_ENV_WIN)
	WSADATA WsaData;
	WSAStartup (0x0202, &WsaData);
#endif

	SOCKET ServerSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT (ServerSock != INVALID_SOCKET);

	sockaddr_in SockAddr = { 0 };
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons (m_pCmdLine->m_ServerPort);

	Result = bind (ServerSock, (sockaddr*) &SockAddr, sizeof (SockAddr));
	if (Result == -1)
	{
		m_pOutStream->Printf (
			"cannot open TCP port %d (%s)\n",
			m_pCmdLine->m_ServerPort,
			err::CError (getsockerror ()).GetDescription ().cc ()
			);

		return EJncError_IoFailure;
	}

	listen (ServerSock, 1);

	m_pOutStream->Printf ("listening on TCP port %d...\n", m_pCmdLine->m_ServerPort);

	for (;;)
	{
		socklen_t SockAddrSize = sizeof (SockAddr);

		SOCKET ConnSock = accept (ServerSock, (sockaddr*) &SockAddr, &SockAddrSize);
		m_pOutStream->Printf (
			"%s:%d: connection accepted\n",
			inet_ntoa (SockAddr.sin_addr),
			ntohs (SockAddr.sin_port)
			);

		Client (ConnSock, &SockAddr);
		closesocket (ConnSock);
	}

	closesocket (ServerSock);
	return 0;
}

int
CJnc::Client (
	SOCKET Socket,
	sockaddr_in* pSockAddr
	)
{
	int Result;

	rtl::CString CmdLineString;

	for (;;)
	{
		char Buffer [256];
		Result = recv (Socket, Buffer, sizeof (Buffer), 0);
		if (Result <= 0)
		{
			m_pOutStream->Printf (
				"%s:%d: premature connection close\n",
				inet_ntoa (pSockAddr->sin_addr),
				ntohs (pSockAddr->sin_port)
				);
			return EJncError_IoFailure;
		}

		char* p = strnchr (Buffer, Result, '\n');
		if (p)
		{
			*p = '\0';
			CmdLineString.Append (Buffer, p - Buffer);
			break;
		}

		CmdLineString.Append (Buffer, Result);
	}

	m_pOutStream->Printf (
		"%s:%d: cmdline: %s\n",
		inet_ntoa (pSockAddr->sin_addr),
		ntohs (pSockAddr->sin_port),
		CmdLineString.cc ()
		);

	CSocketOutStream SocketOut;
	SocketOut.m_Socket = Socket;

	TCmdLine CmdLine;
	CCmdLineParser Parser (&CmdLine);

	Result = Parser.Parse (CmdLineString, CmdLineString.GetLength ());
	if (!Result)
	{
		SocketOut.Printf ("error parsing command line: %s\n", err::GetError ()->GetDescription ().cc ());
		return EJncError_InvalidCmdLine;
	}

	if (CmdLine.m_Flags & EJncFlag_Server)
	{
		SocketOut.Printf ("recursive server request\n");
		return EJncError_InvalidCmdLine;
	}

	CJnc Jnc;
	return Jnc.Run (&CmdLine, &SocketOut);
}

//.............................................................................
