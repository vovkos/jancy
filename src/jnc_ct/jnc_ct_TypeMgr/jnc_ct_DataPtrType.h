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

class DataPtrType: public Type {
	friend class TypeMgr;

protected:
	Type* m_targetType;
	uint_t m_bitOffset; // PtrTypeFlag_BitField only
	uint_t m_bitCount;  // PtrTypeFlag_BitField only

public:
	DataPtrType();

	DataPtrKind
	getPtrKind() {
		return getDataPtrKindFromFlags(m_flags);
	}

	ConstKind
	getConstKind() {
		return getConstKindFromFlags(m_flags);
	}

	Type*
	getTargetType() {
		return m_targetType;
	}

	size_t
	getBitOffset() {
		return m_bitOffset;
	}

	size_t
	getBitCount() {
		return m_bitCount;
	}

	DataPtrType*
	getSafePtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getDataPtrType(m_typeKind, m_flags & PtrTypeFlag__All | PtrTypeFlag_Safe) :
			this;
	}

	DataPtrType*
	getUnsafePtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getDataPtrType(m_typeKind, m_flags & PtrTypeFlag__All & ~PtrTypeFlag_Safe) :
			this;
	}

	DataPtrType*
	getNonConstPtrType() {
		return (m_flags & PtrTypeFlag__ConstKindMask) ?
			m_targetType->getDataPtrType(m_typeKind, m_flags & PtrTypeFlag__All & ~PtrTypeFlag__ConstKindMask) :
			this;
	}

	static
	sl::String
	createSignature(
		Type* targetType,
		uint_t bitOffset,
		uint_t bitCount,
		TypeKind typeKind,
		uint_t flags
	);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

	sl::StringRef
	getTargetValueString(
		const void* p,
		const char* formatSpec = NULL
	);

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

	virtual
	Type*
	calcFoldedDualType(
		AccessKind accessKind,
		ConstKind constKind
	);

	virtual
	Type*
	mergeAutoConstTypes(Type* constType);

protected:
	virtual
	bool
	resolveImports() {
		return m_targetType->ensureNoImports();
	}

	virtual
	bool
	calcLayout();

	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_targetType, m_bitOffset, m_bitCount, m_typeKind, m_flags);
		m_flags |= m_targetType->getFlags() & TypeFlag_SignatureMask;
	}

	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_DataPtrType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DataPtrType::DataPtrType() {
	m_typeKind = TypeKind_DataPtr;
	m_targetType = NULL;
	m_alignment = sizeof(void*);
	m_bitOffset = 0;
	m_bitCount = 0;
}

//..............................................................................

struct DataPtrTypeTuple: sl::ListLink {
	DataPtrType* m_ptrTypeArray[2][DataPtrKind__Count][ConstKind__Count][2][2]; // ref x ptrkind x constkind x volatile x safe
	DataPtrTypeTuple* m_bigEndianTuple; // same for PtrTypeFlag_BigEndian
};

//..............................................................................

} // namespace ct
} // namespace jnc
