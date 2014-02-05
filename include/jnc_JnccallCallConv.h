// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_JNCCALLCALLCONV

#include "jnc_CallConv_msc.h"
#include "jnc_CallConv_gcc32.h"
#include "jnc_CdeclCallConv_gcc64.h"

namespace jnc {

// currently i dont handle differences between jnccall and cdecl
// so, for now jnccall = cdecl

//.............................................................................

class CJnccallCallConv_msc32: public CCdeclCallConv_msc32
{
public:
	CJnccallCallConv_msc32 ()
	{
		m_CallConvKind = ECallConv_Jnccall_msc32;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CJnccallCallConv_msc64: public CCdeclCallConv_msc64
{
public:
	CJnccallCallConv_msc64 ()
	{
		m_CallConvKind = ECallConv_Jnccall_msc64;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CJnccallCallConv_gcc32: public CCdeclCallConv_gcc32
{
public:
	CJnccallCallConv_gcc32 ()
	{
		m_CallConvKind = ECallConv_Jnccall_gcc32;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CJnccallCallConv_gcc64: public CCdeclCallConv_gcc64
{
public:
	CJnccallCallConv_gcc64 ()
	{
		m_CallConvKind = ECallConv_Jnccall_gcc64;
	}
};

//.............................................................................

} // namespace jnc {
