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

#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

class AutoConstType: public Type {
	friend class TypeMgr;

protected:
	Type* m_originalType;
	Type* m_constType;
	Type* m_mergedType;

public:
	AutoConstType();

	Type*
	getOriginalType() {
		return m_originalType;
	}

	Type*
	getConstType() {
		return m_constType;
	}

	Type*
	getMergedType() {
		return m_mergedType;
	}

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	) {
		m_constType->markGcRoots(p, gcHeap);
	}

	static
	uint_t
	createSignature(
		sl::String* signature,
		Type* originalType,
		Type* constType,
		uint_t flags
	);

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	void
	prepareSignature();

	virtual
	void
	prepareLlvmType() {
		m_llvmType = m_constType->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType() {
		m_llvmDiType = m_constType->getLlvmDiType();
	}

	virtual
	Type*
	calcFoldedDualType(
		bool isAlien,
		ConstKind constKind
	);

	virtual
	bool
	calcLayout();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
AutoConstType::AutoConstType() {
	m_typeKind = TypeKind_AutoConst;
	m_originalType = NULL;
	m_constType = NULL;
	m_mergedType = NULL;
}

//..............................................................................

inline
Type*
Type::getActualTypeIfAutoConst() {
	if (m_typeKind != TypeKind_AutoConst)
		return this;

	AutoConstType* autoConstType = (AutoConstType*)this;
	return (m_flags & TypeFlag_Dual) ?
		autoConstType->getConstType() :
		autoConstType->getOriginalType();
}

//..............................................................................

} // namespace ct
} // namespace jnc
