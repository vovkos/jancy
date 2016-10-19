// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_CallConv.h"

namespace jnc {
namespace ct {

// surpisingly enough, by default LLVM produces MSC-compatible (not GCC!) callconv

//..............................................................................

class CdeclCallConv_msc32: public CallConv
{
public:
	CdeclCallConv_msc32 ()
	{
		m_callConvKind = CallConvKind_Cdecl_msc32;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class StdcallCallConv_msc32: public CallConv
{
public:
	StdcallCallConv_msc32 ()
	{
		m_callConvKind = CallConvKind_Stdcall_msc32;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class ThiscallCallConv_msc32: public CallConv
{
public:
	ThiscallCallConv_msc32 ()
	{
		m_callConvKind = CallConvKind_Thiscall_msc32;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
