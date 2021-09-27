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

#include "jnc_rtl_ModuleItem.h"
#include "jnc_ct_Function.h"
#include "jnc_ct_FunctionOverload.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Function)
JNC_DECLARE_OPAQUE_CLASS_TYPE(FunctionOverload)

//..............................................................................

class Function:
	public ModuleItemBase<ct::Function>,
	public ModuleItemDecl {
public:
	Function(ct::Function* function):
		ModuleItemBase(function),
		ModuleItemDecl(function) {}

	FunctionKind
	JNC_CDECL
	getFunctionKind() {
		return m_item->getFunctionKind();
	}

	FunctionType*
	JNC_CDECL
	getType() {
		return (FunctionType*)rtl::getType(m_item->getType());
	}

	bool
	JNC_CDECL
	isMember() {
		return m_item->isMember();
	}

	void*
	JNC_CDECL
	getMachineCode() {
		return m_item->getMachineCode();
	}
};

//..............................................................................

class FunctionOverload:
	public ModuleItemBase<ct::FunctionOverload>,
	public ModuleItemDecl {
public:
	FunctionOverload(ct::FunctionOverload* overload):
		ModuleItemBase(overload),
		ModuleItemDecl(overload) {}

	FunctionKind
	JNC_CDECL
	getFunctionKind() {
		return m_item->getFunctionKind();
	}

	size_t
	JNC_CDECL
	getOverloadCount() {
		return m_item->getOverloadCount();
	}

	Function*
	JNC_CDECL
	getOverload(size_t index) {
		return rtl::getFunction(m_item->getOverload(index));
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
