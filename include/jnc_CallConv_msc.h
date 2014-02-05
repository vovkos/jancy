// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CALLCONV_MSC

#include "jnc_CallConv.h"

namespace jnc {

// surpisingly enough, by default LLVM produces MSC-compatible (not GCC!) callconv

//.............................................................................

class CCdeclCallConv_msc32: public CCallConv
{
public:
	CCdeclCallConv_msc32 ()
	{
		m_CallConvKind = ECallConv_Cdecl_msc32;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCdeclCallConv_msc64: public CCallConv
{
public:
	CCdeclCallConv_msc64 ()
	{
		m_CallConvKind = ECallConv_Cdecl_msc64;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CStdcallCallConv_msc32: public CCallConv
{
public:
	CStdcallCallConv_msc32 ()
	{
		m_CallConvKind = ECallConv_Stdcall_msc32;
	}
};

//.............................................................................

} // namespace jnc {
