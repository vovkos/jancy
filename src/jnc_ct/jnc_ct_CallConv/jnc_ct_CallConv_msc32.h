//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

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
