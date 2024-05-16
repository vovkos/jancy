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
	friend class Parser;

protected:
	StructTypeKind m_structTypeKind;

	size_t m_fieldAlignment;
	size_t m_fieldSize;

	sl::Array<llvm::Type*> m_llvmFieldTypeArray;
	Field* m_lastBitField;
	size_t m_dynamicStructSectionId;

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
	getFieldSize() {
		return m_fieldSize;
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
		size_t bitCount,
		uint_t ptrTypeFlags,
		sl::List<Token>* constructor,
		sl::List<Token>* initializer
	);

	virtual
	void
	prepareSignature() {
		m_signature = 'S' + m_qualifiedName;
		m_flags |= TypeFlag_SignatureFinal;
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
	layoutFieldImpl(
		Type* type,
		size_t* offset,
		uint_t* llvmIndex
	);

	bool
	layoutBitField(Field* field);

	size_t
	getFieldOffset(Type* type);

	void
	addLlvmPadding(size_t size);
};

//..............................................................................

} // namespace ct
} // namespace jnc
