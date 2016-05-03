#include "pch.h"
#include "Jnc.h"

//.............................................................................

int
JncLib::printf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);

	Jnc* jnc = Jnc::getCurrentJnc ();
	ASSERT (jnc);

	return jnc->getOutStream ()->printf_va (format, va);
}

//.............................................................................

Jnc* Jnc::m_currentJnc = NULL;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
Jnc::run (
	CmdLine* cmdLine,
	OutStream* outStream
	)
{
	bool result;

	m_cmdLine = cmdLine;
	m_outStream = outStream;

	if (cmdLine->m_flags & JncFlag_Help)
	{
		printUsage (outStream);
		return JncError_Success;
	}

	if (cmdLine->m_flags & JncFlag_Version)
	{
		printVersion (outStream);
		return JncError_Success;
	}

	if (cmdLine->m_flags & JncFlag_Server)
		return server ();

	result = compile ();
	if (!result)
	{
		outStream->printf ("%s\n", err::getLastErrorDescription ().cc ());
		return JncError_CompileFailure;
	}

	if (cmdLine->m_flags & JncFlag_LlvmIr)
		printLlvmIr ();

	if (m_cmdLine->m_flags & JncFlag_Jit)
	{
		result = jit ();
		if (!result)
		{
			outStream->printf ("%s\n", err::getLastErrorDescription ().cc ());
			return JncError_CompileFailure;
		}
	}

	if (!(cmdLine->m_flags & JncFlag_CompileOnly))
	{
		m_runtime.setStackSizeLimit (cmdLine->m_stackSizeLimit);
		m_runtime.m_gcHeap.setSizeTriggers (
			cmdLine->m_gcAllocSizeTrigger,
			cmdLine->m_gcPeriodSizeTrigger
			);

		int returnValue;
		result = runFunction (&returnValue);
		if (!result)
		{
			outStream->printf ("%s\n", err::getLastErrorDescription ().cc ());
			return JncError_RunFailure;
		}

		if (!(cmdLine->m_flags & JncFlag_PrintReturnValue))
			return returnValue;

		printf ("'%s' returned: %d\n", cmdLine->m_functionName.cc (), returnValue);
	}

	return JncError_Success;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Jnc::compile ()
{
	bool result;

	uint_t compileFlags = jnc::ct::ModuleCompileFlag_StdFlags;
	if (m_cmdLine->m_flags & JncFlag_DebugInfo)
		compileFlags |= jnc::ct::ModuleCompileFlag_DebugInfo;

	if (m_cmdLine->m_flags & JncFlag_McJit)
		compileFlags |= jnc::ct::ModuleCompileFlag_McJit;

	if (m_cmdLine->m_flags & JncFlag_SimpleGcSafePoint)
		compileFlags |= jnc::ct::ModuleCompileFlag_SimpleGcSafePoint;

	jnc::ext::ExtensionLibHost* libHost = jnc::ext::getStdExtensionLibHost ();

	result = 
		m_module.create ("jnc_module", compileFlags) &&
		m_module.m_extensionLibMgr.addStaticLib (jnc::ext::getStdLib (libHost)) &&
		m_module.m_extensionLibMgr.addStaticLib (jnc::ext::getSysLib (libHost)) &&
		m_module.m_extensionLibMgr.addStaticLib (sl::getSimpleSingleton <JncLib> ());

	if (!result)
		return false;

	m_module.m_importMgr.m_importDirList.copy (m_cmdLine->m_importDirList);

	if (m_cmdLine->m_flags & JncFlag_StdInSrc)
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		int stdInFile = _fileno (stdin);
#endif
		sl::Array <char> stdInBuffer;

		for (;;)
		{
			char buffer [1024];
#if (_AXL_ENV == AXL_ENV_WIN)
			int result = _read (stdInFile, buffer, sizeof (buffer));
#else
			int result = read (STDIN_FILENO, buffer, sizeof (buffer));
#endif
			if (result <= 0)
				break;

			stdInBuffer.append (buffer, result);
		}

		const char* srcName = !m_cmdLine->m_srcNameOverride.isEmpty () ?
			m_cmdLine->m_srcNameOverride.cc () :
			"stdin";

		result = m_module.parse (srcName, stdInBuffer, stdInBuffer.getCount ());
		if (!result)
			return false;
	}
	else
	{
		sl::BoxIterator <sl::String> fileNameIt = m_cmdLine->m_fileNameList.getHead ();
		ASSERT (fileNameIt);

		if (!m_cmdLine->m_srcNameOverride.isEmpty ())
		{
			result = m_module.parseFile (*fileNameIt, m_cmdLine->m_srcNameOverride);
			if (!result)
				return false;

			fileNameIt++;
		}

		for (; fileNameIt; fileNameIt++)
		{
			result = m_module.parseFile (*fileNameIt);
			if (!result)
				return false;
		}
	}

	return 
		m_module.parseImports () &&
		m_module.compile ();
}

bool
Jnc::jit ()
{
	return m_module.jit ();
}

void
Jnc::printLlvmIr ()
{
	m_outStream->printf ("%s", m_module.getLlvmIrString ().cc ());
}

bool
Jnc::runFunction (int* returnValue)
{
	bool result;

	jnc::ct::ModuleItem* functionItem = m_module.m_namespaceMgr.getGlobalNamespace ()->findItem (m_cmdLine->m_functionName);
	if (!functionItem || functionItem->getItemKind () != jnc::ct::ModuleItemKind_Function)
	{
		err::setFormatStringError ("'%s' is not found or not a function\n", m_cmdLine->m_functionName.cc ());
		return false;
	}

	jnc::ct::Function* function = (jnc::ct::Function*) functionItem;
	jnc::ct::FunctionType* functionType = function->getType ();
	jnc::ct::TypeKind returnTypeKind = functionType->getReturnType ()->getTypeKind ();
	size_t argCount = functionType->getArgArray ().getCount ();
	if (returnTypeKind != jnc::ct::TypeKind_Void && returnTypeKind != jnc::ct::TypeKind_Int || argCount)
	{
		err::setFormatStringError ("'%s' has invalid signature: %s\n", m_cmdLine->m_functionName.cc (), functionType->getTypeString ().cc ());
		return false;
	}

	result = m_runtime.startup (&m_module);
	if (!result)
		return false;

	if (returnTypeKind == jnc::ct::TypeKind_Int)
	{
		jnc::rt::callFunction (&m_runtime, function, returnValue);
	}
	else
	{
		jnc::rt::callVoidFunction (&m_runtime, function);
		*returnValue = 0;
	}

	if (!result)
		return false;

	m_runtime.shutdown ();

	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
Jnc::server ()
{
	int result;

	printVersion (m_outStream);

#if (_AXL_ENV == AXL_ENV_WIN)
	WSADATA wsaData;
	WSAStartup (0x0202, &wsaData);
#endif

	SOCKET serverSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT (serverSock != INVALID_SOCKET);

	sockaddr_in sockAddr = { 0 };
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons (m_cmdLine->m_serverPort);

	result = bind (serverSock, (sockaddr*) &sockAddr, sizeof (sockAddr));
	if (result == -1)
	{
		m_outStream->printf (
			"cannot open TCP port %d (%s)\n",
			m_cmdLine->m_serverPort,
			err::Error (getsockerror ()).getDescription ().cc ()
			);

		return JncError_IoFailure;
	}

	listen (serverSock, 1);

	m_outStream->printf ("listening on TCP port %d...\n", m_cmdLine->m_serverPort);

	for (;;)
	{
		socklen_t sockAddrSize = sizeof (sockAddr);

		SOCKET connSock = accept (serverSock, (sockaddr*) &sockAddr, &sockAddrSize);
		m_outStream->printf (
			"%s:%d: connection accepted\n",
			inet_ntoa (sockAddr.sin_addr),
			ntohs (sockAddr.sin_port)
			);

		client (connSock, &sockAddr);
		closesocket (connSock);
	}

	closesocket (serverSock);
	return 0;
}

int
Jnc::client (
	SOCKET socket,
	sockaddr_in* sockAddr
	)
{
	int result;

	sl::String cmdLineString;

	for (;;)
	{
		char buffer [256];
		result = recv (socket, buffer, sizeof (buffer), 0);
		if (result <= 0)
		{
			m_outStream->printf (
				"%s:%d: premature connection close\n",
				inet_ntoa (sockAddr->sin_addr),
				ntohs (sockAddr->sin_port)
				);
			return JncError_IoFailure;
		}

		char* p = strnchr (buffer, result, '\n');
		if (p)
		{
			*p = '\0';
			cmdLineString.append (buffer, p - buffer);
			break;
		}

		cmdLineString.append (buffer, result);
	}

	m_outStream->printf (
		"%s:%d: cmdline: %s\n",
		inet_ntoa (sockAddr->sin_addr),
		ntohs (sockAddr->sin_port),
		cmdLineString.cc ()
		);

	SocketOutStream socketOut;
	socketOut.m_socket = socket;

	CmdLine cmdLine;
	CmdLineParser parser (&cmdLine);

	result = parser.parse (cmdLineString, cmdLineString.getLength ());
	if (!result)
	{
		socketOut.printf ("error parsing command line: %s\n", err::getLastErrorDescription ().cc ());
		return JncError_InvalidCmdLine;
	}

	if (cmdLine.m_flags & JncFlag_Server)
	{
		socketOut.printf ("recursive server request\n");
		return JncError_InvalidCmdLine;
	}

	Jnc jnc;
	return jnc.run (&cmdLine, &socketOut);
}

//.............................................................................

