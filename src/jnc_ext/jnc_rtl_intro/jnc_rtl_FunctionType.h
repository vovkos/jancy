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

#include "jnc_rtl_Type.h"
#include "jnc_ct_FunctionType.h"
#include "jnc_ct_FunctionPtrType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(FunctionArg)
JNC_DECLARE_OPAQUE_CLASS_TYPE(FunctionType)
JNC_DECLARE_OPAQUE_CLASS_TYPE(FunctionPtrType)

//..............................................................................

class FunctionArg:
	public ModuleItemBase<ct::FunctionArg>,
	public ModuleItemDecl,
	public ModuleItemInitializer {
public:
	FunctionArg(ct::FunctionArg* variable):
		ModuleItemBase(variable),
		ModuleItemDecl(variable),
		ModuleItemInitializer(variable) {}

	Type*
	JNC_CDECL
	getType() {
		return rtl::getType(m_item->getType());
	}

	uint_t
	JNC_CDECL
	getPtrTypeFlags() {
		return m_item->getPtrTypeFlags();
	}
};

//..............................................................................

class FunctionType: public TypeBase<ct::FunctionType> {
public:
	FunctionType(ct::FunctionType* type):
		TypeBase(type) {}

	Type*
	JNC_CDECL
	getReturnType() {
		return rtl::getType(m_item->getReturnType());
	}

	size_t
	JNC_CDECL
	getArgCount() {
		return m_item->getArgArray().getCount();
	}

	FunctionArg*
	JNC_CDECL
	getArg(size_t index) {
		size_t count = m_item->getArgArray().getCount();
		return index < count ? rtl::getFunctionArg(m_item->getArgArray()[index]) : NULL;
	}

	FunctionType*
	JNC_CDECL
	getShortType() {
		return (FunctionType*)rtl::getType(m_item->getShortType());
	}

	FunctionPtrType*
	JNC_CDECL
	getFunctionPtrType(
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind,
		uint_t flags
	) {
		return (FunctionPtrType*)rtl::getType(m_item->getFunctionPtrType(typeKind, ptrTypeKind, flags));
	}
};

//..............................................................................

class FunctionPtrType: public TypeBase<ct::FunctionPtrType> {
public:
	FunctionPtrType(ct::FunctionPtrType* type):
		TypeBase(type) {}

	FunctionPtrTypeKind
	JNC_CDECL
	getPtrTypeKind() {
		return m_item->getPtrTypeKind();
	}

	FunctionType*
	JNC_CDECL
	getTargetType() {
		return (FunctionType*)rtl::getType(m_item->getTargetType());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
