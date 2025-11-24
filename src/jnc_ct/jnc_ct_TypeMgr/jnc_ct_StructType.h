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

#include "jnc_StructType.h"
#include "jnc_ct_DerivableType.h"
#include "jnc_ct_Pragma.h"

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
	size_t m_laidOutFieldCount;

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

	bool
	ensureLayoutTo(Field* field) {
		return (field->m_flags & FieldFlag_LayoutReady) ? true : calcLayoutTo(field);
	}

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
	calcLayout() {
		return calcLayoutTo(NULL);
	}

	bool
	calcLayoutTo(Field* field); // NULL means full layout

	bool
	layoutBaseType(BaseTypeSlot* slot);

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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
StructType::StructType() {
	m_typeKind = TypeKind_Struct;
	m_structTypeKind = StructTypeKind_Normal;
	m_flags = TypeFlag_Pod | TypeFlag_StructRet;
	m_fieldAlignment = PragmaDefault_Alignment;
	m_fieldSize = 0;
	m_laidOutFieldCount = 0;
	m_lastBitField = NULL;
	m_dynamicStructSectionId = -1;
}

//..............................................................................

} // namespace ct
} // namespace jnc
