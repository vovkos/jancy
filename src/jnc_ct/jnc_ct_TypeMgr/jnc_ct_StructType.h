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

#include "jnc_ct_DerivableType.h"
#include "jnc_ct_BitFieldType.h"

namespace jnc {
namespace ct {

//..............................................................................

enum StructTypeKind {
	StructTypeKind_Normal,
	StructTypeKind_IfaceStruct,
	StructTypeKind_ClassStruct,
	StructTypeKind_UnionStruct,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class StructType: public DerivableType {
	friend class TypeMgr;
	friend class ClassType;
	friend class UnionType;
	friend class Property;
	friend class AsyncSequencerFunction;

protected:
	StructTypeKind m_structTypeKind;

	size_t m_fieldAlignment;
	size_t m_fieldActualSize;
	size_t m_fieldAlignedSize;

	sl::Array<Field*> m_dynamicFieldArray;
	sl::Array<llvm::Type*> m_llvmFieldTypeArray;
	BitFieldType* m_lastBitFieldType;
	size_t m_lastBitFieldOffset;

public:
	StructType();

	StructTypeKind
	getStructTypeKind() {
		return m_structTypeKind;
	}

	size_t
	getFieldAlignment() {
		return m_fieldAlignment;
	}

	size_t
	getFieldActualSize() {
		return m_fieldActualSize;
	}

	size_t
	getFieldAlignedSize() {
		return m_fieldAlignedSize;
	}

	sl::Array<Field*>
	getDynamicFieldArray() {
		return m_dynamicFieldArray;
	}

	bool
	append(StructType* type);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

protected:
	virtual
	Field*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList<Token>* constructor = NULL,
		sl::BoxList<Token>* initializer = NULL
	);

	virtual
	void
	prepareSignature() {
		m_signature = 'S' + m_qualifiedName;
	}

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_StructType);
	}

	virtual
	bool
	calcLayout();

	bool
	layoutField(Field* field);

	bool
	layoutField(
		Type* type,
		size_t* offset,
		uint_t* llvmIndex
	);

	bool
	layoutBitField(
		Type* baseType,
		size_t bitCount,
		Type** type,
		size_t* offset,
		uint_t* llvmIndex
	);

	size_t
	getFieldOffset(size_t alignment);

	size_t
	setFieldActualSize(size_t size);

	void
	addLlvmPadding(size_t size);
};

//..............................................................................

} // namespace ct
} // namespace jnc
