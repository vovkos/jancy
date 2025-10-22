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
#include "jnc_ct_EnumType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(EnumConst)
JNC_DECLARE_OPAQUE_CLASS_TYPE(EnumType)

//..............................................................................

class EnumConst:
	public ModuleItemBase<ct::EnumConst>,
	public ModuleItemDecl,
	public ModuleItemInitializer {
protected:
	String m_name;

public:
	EnumConst(ct::EnumConst* cnst):
		ModuleItemBase(cnst),
		ModuleItemDecl(cnst),
		ModuleItemInitializer(cnst) {}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	EnumType*
	JNC_CDECL
	getParentType() {
		return (EnumType*)rtl::getType(m_item->getParentEnumType());
	}

	static
	String
	JNC_CDECL
	getName(EnumConst* self);

	int64_t
	JNC_CDECL
	getValue() {
		return m_item->getValue();
	}
};

//..............................................................................

class EnumType: public NamedTypeBase<ct::EnumType> {
public:
	EnumType(ct::EnumType* type):
		NamedTypeBase(type) {}

	Type*
	JNC_CDECL
	getBaseType() {
		return rtl::getType(m_item->getBaseType());
	}

	size_t
	JNC_CDECL
	getConstCount() {
		return m_item->getConstArray().getCount();
	}

	EnumConst*
	JNC_CDECL
	getConst(size_t index) {
		size_t count = m_item->getConstArray().getCount();
		return index < count ? rtl::getEnumConst(m_item->getConstArray()[index]) : NULL;
	}

	EnumConst*
	JNC_CDECL
	findConst(int64_t value) {
		jnc::EnumConst* enumConst = m_item->findConst(value);
		return enumConst ? rtl::getEnumConst(enumConst) : NULL;
	}

	static
	String
	JNC_CDECL
	getValueName_0(
		EnumType* self,
		DataPtr valuePtr,
		String formatSpec
	);

	static
	String
	JNC_CDECL
	getValueName_1(
		EnumType* self,
		Variant value,
		String formatSpec
	);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
