// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DerivableType.h"
#include "jnc_BitFieldType.h"

namespace jnc {

class StructType;
class UnionType;
struct FmtLiteral;

//.............................................................................

class StructField:
	public UserModuleItem,
	public ModuleItemInitializer
{
	friend class TypeMgr;
	friend class NamedTypeBlock;
	friend class DerivableType;
	friend class Property;
	friend class StructType;
	friend class UnionType;
	friend class ClassType;

protected:
	Type* m_type;
	ImportType* m_type_i;
	uint_t m_ptrTypeFlags;
	rtl::BoxList <Token> m_constructor;

	Type* m_bitFieldBaseType;
	size_t m_bitCount;
	size_t m_offset;
	uint_t m_llvmIndex;

public:
	StructField ();

	Type*
	getType ()
	{
		return m_type;
	}

	ImportType*
	getType_i ()
	{
		return m_type_i;
	}

	int
	getPtrTypeFlags ()
	{
		return m_ptrTypeFlags;
	}

	rtl::ConstBoxList <Token>
	getConstructor ()
	{
		return m_constructor;
	}

	size_t
	getOffset ()
	{
		return m_offset;
	}

	uint_t
	getLlvmIndex ()
	{
		return m_llvmIndex;
	}
};

//.............................................................................

enum StructTypeKind
{
	StructTypeKind_Normal,
	StructTypeKind_IfaceStruct,
	StructTypeKind_ClassStruct,
	StructTypeKind_UnionStruct,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class StructType: public DerivableType
{
	friend class TypeMgr;
	friend class ClassType;
	friend class UnionType;
	friend class Property;

protected:
	StructTypeKind m_structTypeKind;

	size_t m_fieldAlignment;
	size_t m_fieldActualSize;
	size_t m_fieldAlignedSize;

	rtl::Array <llvm::Type*> m_llvmFieldTypeArray;
	BitFieldType* m_lastBitFieldType;
	size_t m_lastBitFieldOffset;

public:
	StructType ();

	StructTypeKind
	getStructTypeKind ()
	{
		return m_structTypeKind;
	}

	size_t
	getFieldAlignment ()
	{
		return m_fieldAlignment;
	}

	size_t
	getFieldActualSize ()
	{
		return m_fieldActualSize;
	}

	size_t
	getFieldAlignedSize ()
	{
		return m_fieldAlignedSize;
	}

	bool
	append (StructType* type);

	virtual
	bool
	compile ();

	virtual
	void
	markGcRoots (
		const void* p,
		GcHeap* gcHeap
		);

protected:
	virtual
	StructField*
	createFieldImpl (
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		);

	virtual
	void
	prepareTypeString ()
	{
		m_typeString.format ("struct %s", m_tag.cc ()); // thanks a lot gcc
	}

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();

	virtual
	bool
	calcLayout ();

	bool
	layoutField (
		llvm::Type* llvmType,
		size_t size,
		size_t alignment,
		size_t* offset,
		uint_t* llvmIndex
		);

	bool
	layoutField (
		Type* type,
		size_t* offset,
		uint_t* llvmIndex
		)
	{
		return
			type->ensureLayout () &&
			layoutField (
				type->getLlvmType (),
				type->getSize (),
				type->getAlignment (),
				offset,
				llvmIndex
				);
	}

	bool
	layoutBitField (
		Type* baseType,
		size_t bitCount,
		Type** type,
		size_t* offset,
		uint_t* llvmIndex
		);

	size_t
	getFieldOffset (size_t alignment);

	size_t
	setFieldActualSize (size_t size);

	ArrayType*
	insertPadding (size_t size);
};

//.............................................................................

} // namespace jnc {
