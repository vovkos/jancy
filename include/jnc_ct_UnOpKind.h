// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

//.............................................................................

enum UnOpKind
{
	UnOpKind_Undefined = 0,
	UnOpKind_Plus,
	UnOpKind_Minus,
	UnOpKind_BwNot,	
	UnOpKind_Addr,
	UnOpKind_Indir,	
	UnOpKind_LogNot,
	UnOpKind_PreInc,
	UnOpKind_PreDec,
	UnOpKind_PostInc,
	UnOpKind_PostDec,	
	UnOpKind_Ptr,
	UnOpKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getUnOpKindString (UnOpKind opKind);

//.............................................................................

} // namespace ct
} // namespace jnc
