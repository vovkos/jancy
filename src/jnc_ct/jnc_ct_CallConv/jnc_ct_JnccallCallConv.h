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

#include "jnc_ct_CallConv_msc32.h"
#include "jnc_ct_CallConv_gcc32.h"
#include "jnc_ct_CdeclCallConv_gcc64.h"
#include "jnc_ct_CdeclCallConv_msc64.h"
#include "jnc_ct_CdeclCallConv_arm.h"

namespace jnc {
namespace ct {

// currently i dont handle differences between jnccall and cdecl
// so, for now jnccall = cdecl

//..............................................................................

class JnccallCallConv_msc32: public CdeclCallConv_msc32
{
public:
	JnccallCallConv_msc32 ()
	{
		m_callConvKind = CallConvKind_Jnccall_msc32;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class JnccallCallConv_msc64: public CdeclCallConv_msc64
{
public:
	JnccallCallConv_msc64 ()
	{
		m_callConvKind = CallConvKind_Jnccall_msc64;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class JnccallCallConv_gcc32: public CdeclCallConv_gcc32
{
public:
	JnccallCallConv_gcc32 ()
	{
		m_callConvKind = CallConvKind_Jnccall_gcc32;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class JnccallCallConv_gcc64: public CdeclCallConv_gcc64
{
public:
	JnccallCallConv_gcc64 ()
	{
		m_callConvKind = CallConvKind_Jnccall_gcc64;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class JnccallCallConv_arm32: public CdeclCallConv_arm32
{
public:
	JnccallCallConv_arm32 ()
	{
		m_callConvKind = CallConvKind_Jnccall_arm32;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class JnccallCallConv_arm64: public CdeclCallConv_arm64
{
public:
	JnccallCallConv_arm64 ()
	{
		m_callConvKind = CallConvKind_Jnccall_arm64;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
