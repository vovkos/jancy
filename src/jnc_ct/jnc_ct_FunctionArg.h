// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ImportType.h"
#include "jnc_ct_Scope.h"

namespace jnc {
namespace ct {

class Scope;

//..............................................................................

class FunctionArg:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
	friend class TypeMgr;
	friend class Function;
	friend class ClassType;
	friend class Orphan;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;

public:
	FunctionArg ();

	Type*
	getType ()
	{
		return m_type;
	}

	uint_t
	getPtrTypeFlags ()
	{
		return m_ptrTypeFlags;
	}

	sl::String
	getArgString ();

	sl::String
	getArgDoxyLinkedText ();
};

//..............................................................................

struct FunctionArgTuple: sl::ListLink
{
	FunctionArg* m_argArray [2] [2] [2]; // this x const x volatile
};

//..............................................................................

} // namespace ct
} // namespace jnc
