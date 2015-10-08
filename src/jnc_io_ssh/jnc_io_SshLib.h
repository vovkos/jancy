// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_io_Ssh.h"
#include "jnc_io_Ssh.jnc.cpp"

namespace jnc {
namespace io {

//.............................................................................

class SshLib: public ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_TYPE (SshChannel)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (SshChannel)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

	JNC_BEGIN_LIB_SOURCE_FILE_TABLE ()
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("io_Ssh.jnc", g_io_sshSrc)
	JNC_END_LIB_SOURCE_FILE_TABLE ()

	JNC_BEGIN_LIB_FORCED_EXPORT ()
		JNC_LIB_FORCED_SOURCE_FILE ("io_Ssh.jnc", g_io_sshSrc)
	JNC_END_LIB_FORCED_EXPORT ()
};

//.............................................................................

} // namespace io
} // namespace jnc
