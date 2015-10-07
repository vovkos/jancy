#pragma once

#include "jnc_ct_Pch.h"

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

#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_rt_CallSite.h"
#include "jnc_ext_ExtensionLib.h"

using namespace axl;

//.............................................................................
