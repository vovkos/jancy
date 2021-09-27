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

#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace ct {

class Type;

//..............................................................................

class FunctionArg:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class TypeMgr;
	friend class Function;
	friend class ClassType;
	friend class Orphan;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;

public:
	FunctionArg();

	Type*
	getType() {
		return m_type;
	}

	uint_t
	getPtrTypeFlags() {
		return m_ptrTypeFlags;
	}

	sl::String
	getArgString();

	sl::String
	getArgDoxyLinkedText();
};

//..............................................................................

struct FunctionArgTuple: sl::ListLink {
	FunctionArg* m_argArray[2][2][2]; // this x const x volatile
};

//..............................................................................

} // namespace ct
} // namespace jnc
