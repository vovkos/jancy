#pragma once

#include "jnc_Pch.h"

#if (_AXL_ENV == AXL_ENV_WIN)
#	define getsockerror WSAGetLastError
#	define socklen_t    int
#	include <io.h>
#	include <fcntl.h>

#elif (_AXL_ENV == AXL_ENV_POSIX)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#	include <arpa/inet.h>
#	define SOCKET         int
#	define INVALID_SOCKET (-1)
#	define closesocket    close

inline
int
getsockerror ()
{
	return errno;
}

#endif

//.............................................................................

// Jancy

#include "jnc_StdLib.h"
#include "jnc_Disassembler.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_CallFunction.h"

using namespace axl;

//.............................................................................
