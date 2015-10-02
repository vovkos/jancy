// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Socket.h"
#include "jnc_Serial.h"
#include "jnc_File.h"

#include "jnc_Socket.jnc.cpp"
#include "jnc_Serial.jnc.cpp"
#include "jnc_File.jnc.cpp"

namespace jnc {

//.............................................................................

class IoLib: public ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_TYPE (Socket)
		JNC_MAP_TYPE (Serial)
		JNC_MAP_TYPE (File)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_SOURCE_FILE_TABLE ()
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_Socket.jnc", g_jnc_SocketSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_Serial.jnc", g_jnc_SerialSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_File.jnc",   g_jnc_FileSrc)
	JNC_END_LIB_SOURCE_FILE_TABLE ()
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ExtensionLib* 
getIoLib (ExtensionLibSlotDb* slotDb);

//.............................................................................

} // namespace jnc {
