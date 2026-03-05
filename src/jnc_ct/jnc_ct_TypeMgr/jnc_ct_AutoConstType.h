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

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	) {
		m_constType->markGcRoots(p, gcHeap);
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	void
	prepareSignature() {
		m_signature = ((m_flags & TypeFlag_Dual) ? "ZA" : "ZM") + m_originalType->getSignature() + m_constType->getSignature();
		m_flags |= m_originalType->getFlags() & m_constType->getFlags() & TypeFlag_SignatureMask;
	}

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
		uint_t ptrFlags
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
}

//..............................................................................

} // namespace ct
} // namespace jnc
