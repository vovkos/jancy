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

#include "jnc_Type.h"
#include "jnc_ct_Namespace.h"
#include "jnc_ct_StdType.h"

namespace jnc {
namespace rt {

class GcHeap;

} // namespace rt

namespace ct {

class TypeMgr;
class ArrayType;
class StructType;
class ClassType;
class PropertyType;
class DataPtrType;
class TypedefShadowType;
class ImportType;
class FunctionArg;
class Value;

struct DataPtrTypeTuple;
struct SimplePropertyTypeTuple;
struct FunctionArgTuple;

//..............................................................................

enum TypeSizeLimit
{
	TypeSizeLimit_StoreSize      = 64,
	TypeSizeLimit_StackAllocSize = 128,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum TypeFlag
{
	TypeFlag_Named           = 0x0100,
	TypeFlag_Child           = 0x0200, // constructor has an implicit 'parent' arg
	TypeFlag_Pod             = 0x0400, // plain-old-data
	TypeFlag_GcRoot          = 0x0800, // is or contains gc-traceable pointers
	TypeFlag_StructRet       = 0x1000, // return through hidden 1st arg (gcc32 callconv)
	TypeFlag_NoStack         = 0x2000, // try to avoid allocation on stack
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum PtrTypeFlag
{
	PtrTypeFlag_Safe      = 0x0010000, // all ptr
	PtrTypeFlag_Unused    = 0x0020000, // all ptr
	PtrTypeFlag_Const     = 0x0040000, // class & data ptr
	PtrTypeFlag_ReadOnly  = 0x0080000, // class & data ptr
	PtrTypeFlag_Volatile  = 0x0100000, // class & data ptr
	PtrTypeFlag_Event     = 0x0200000, // multicast-class only
	PtrTypeFlag_DualEvent = 0x0400000, // multicast-class only
	PtrTypeFlag_Bindable  = 0x0800000, // multicast-class only
	PtrTypeFlag_AutoGet   = 0x1000000, // data ptr only

	PtrTypeFlag__AllMask  = 0x1ff0000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
PtrTypeFlag
getFirstPtrTypeFlag (uint_t flags)
{
	return (PtrTypeFlag) (1 << sl::getLoBitIdx (flags));
}

const char*
getPtrTypeFlagString (PtrTypeFlag flag);

sl::String
getPtrTypeFlagString (uint_t flags);

sl::String
getPtrTypeFlagSignature (uint_t flags);

JNC_INLINE
const char*
getFirstPtrTypeFlagString (uint_t flags)
{
	return getPtrTypeFlagString (getFirstPtrTypeFlag (flags));
}

uint_t
getPtrTypeFlagsFromModifiers (uint_t modifiers);

//..............................................................................

enum VariantField
{
	VariantField_Data1,
	VariantField_Data2,
#if (JNC_PTR_SIZE == 4)
	VariantField_Padding,
#endif
	VariantField_Type,
};

//..............................................................................

// integer type utils

TypeKind
getInt32TypeKind (int32_t integer);

TypeKind
getInt32TypeKind_u (uint32_t integer);

TypeKind
getInt64TypeKind (int64_t integer);

TypeKind
getInt64TypeKind_u (uint64_t integer);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
TypeKind
getBigEndianIntegerTypeKind (TypeKind typeKind)
{
	return !(getTypeKindFlags (typeKind) & TypeKindFlag_BigEndian) ?
		(TypeKind) (typeKind + TypeKind__EndianDelta) :
		typeKind;
}

JNC_INLINE
TypeKind
getLittleEndianIntegerTypeKind (TypeKind typeKind)
{
	return (getTypeKindFlags (typeKind) & TypeKindFlag_BigEndian) ?
		(TypeKind) (typeKind - TypeKind__EndianDelta) :
		typeKind;
}

JNC_INLINE
TypeKind
getUnsignedIntegerTypeKind (TypeKind typeKind)
{
	return !(getTypeKindFlags (typeKind) & TypeKindFlag_Unsigned) ?
		(TypeKind) (typeKind + 1) :
		typeKind;
}

JNC_INLINE
TypeKind
getSignedIntegerTypeKind (TypeKind typeKind)
{
	return (getTypeKindFlags (typeKind) & TypeKindFlag_Unsigned) ?
		(TypeKind) (typeKind - 1) :
		typeKind;
}

JNC_INLINE
bool
isEquivalentIntegerTypeKind (
	TypeKind typeKind1,
	TypeKind typeKind2
	)
{
	return getSignedIntegerTypeKind (typeKind1) == getSignedIntegerTypeKind (typeKind2);
}

//..............................................................................

sl::String
getLlvmTypeString (llvm::Type* llvmType);

//..............................................................................

struct TypeStringTuple
{
	sl::String m_typeStringPrefix;
	sl::String m_typeStringSuffix;
	sl::String m_doxyTypeString;
	sl::String m_doxyLinkedTextPrefix;
	sl::String m_doxyLinkedTextSuffix;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Type: public ModuleItem
{
	friend class TypeMgr;
	friend class CdeclCallConv_gcc64;

protected:
	TypeKind m_typeKind;
	StdType m_stdType;
	size_t m_size;
	size_t m_alignment;
	sl::StringHashTableMapIterator <Type*> m_typeMapIt;
	sl::String m_signature;

	llvm::Type* m_llvmType;
	llvm::DIType* m_llvmDiType;

	TypeStringTuple* m_typeStringTuple;
	SimplePropertyTypeTuple* m_simplePropertyTypeTuple;
	FunctionArgTuple* m_functionArgTuple;
	DataPtrTypeTuple* m_dataPtrTypeTuple;
	ClassType* m_boxClassType;

public:
	Type ();

	~Type ();

	TypeKind
	getTypeKind ()
	{
		return m_typeKind;
	}

	uint_t
	getTypeKindFlags ()
	{
		return jnc::getTypeKindFlags (m_typeKind);
	}

	StdType
	getStdType ()
	{
		return m_stdType;
	}

	size_t
	getSize ()
	{
		return m_size;
	}

	size_t
	getAlignment ()
	{
		return m_alignment;
	}

	const sl::String&
	getSignature ()
	{
		return m_signature;
	}

	sl::String
	getTypeString ();

	const sl::String&
	getTypeStringPrefix ();

	const sl::String&
	getTypeStringSuffix ();

	const sl::String&
	getDoxyTypeString ();

	const sl::String&
	getDoxyLinkedTextPrefix ();

	const sl::String&
	getDoxyLinkedTextSuffix ();

	sl::String
	getLlvmTypeString ()
	{
		return ct::getLlvmTypeString (getLlvmType ());
	}

	llvm::Type*
	getLlvmType ();

	llvm::DIType*
	getLlvmDiType ();

	Value
	getUndefValue ();

	Value
	getZeroValue ();

	int
	cmp (Type* type)
	{
		return type != this ? m_signature.cmp (type->m_signature) : 0;
	}

	ArrayType*
	getArrayType (size_t elementCount);

	DataPtrType*
	getDataPtrType (
		Namespace* anchorNamespace,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		);

	DataPtrType*
	getDataPtrType (
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (NULL, typeKind, ptrTypeKind, flags);
	}

	DataPtrType*
	getDataPtrType (
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (TypeKind_DataPtr, ptrTypeKind, flags);
	}

	DataPtrType*
	getDataPtrType_c (
		TypeKind typeKind = TypeKind_DataPtr,
		uint_t flags = 0
		)
	{
		return getDataPtrType (typeKind, DataPtrTypeKind_Thin, flags);
	}

	FunctionArg*
	getSimpleFunctionArg (uint_t ptrTypeFlags = 0);

	virtual
	void
	markGcRoots (
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	TypeStringTuple*
	getTypeStringTuple ();

	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareDoxyTypeString ();

	virtual
	void
	prepareDoxyLinkedText ();

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();

	virtual
	bool
	calcLayout ()
	{
		ASSERT (m_size && m_alignment);
		return true;
	}
};

//..............................................................................

class NamedType:
	public Type,
	public Namespace
{
	friend class Parser;

public:
	NamedType ()
	{
		m_namespaceKind = NamespaceKind_Type;
	}

protected:
	virtual
	void
	prepareTypeString ()
	{
		getTypeStringTuple ()->m_typeStringPrefix = m_tag;
	}

	virtual
	void
	prepareDoxyLinkedText ();
};

//..............................................................................

class Typedef:
	public ModuleItem,
	public ModuleItemDecl
{
	friend class TypeMgr;

protected:
	Type* m_type;
	TypedefShadowType* m_shadowType;

public:
	Typedef ();

	Type*
	getType ()
	{
		return m_type;
	}

	TypedefShadowType*
	getShadowType ();

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TypedefShadowType:
	public Type,
	public ModuleItemDecl
{
	friend class TypeMgr;

protected:
	Typedef* m_typedef;

public:
	TypedefShadowType ()
	{
		m_typeKind = TypeKind_TypedefShadow;
		m_typedef = NULL;
	}

	Typedef*
	getTypedef ()
	{
		return m_typedef;
	}

protected:
	virtual
	void
	prepareTypeString ()
	{
		getTypeStringTuple ()->m_typeStringPrefix = m_tag;
	}

	virtual
	void
	prepareDoxyLinkedText ();

	virtual
	void
	prepareLlvmType ()
	{
		ASSERT (false);
	}

	virtual
	void
	prepareLlvmDiType ()
	{
		ASSERT (false);
	}

	virtual
	bool
	calcLayout ();
};

//..............................................................................

Type*
getSimpleType (
	TypeKind typeKind,
	Module* module
	);

Type*
getSimpleType (
	StdType stdType,
	Module* module
	);

Type*
getModuleItemType (ModuleItem* item);

Type*
getDirectRefType (
	Namespace* anchorNamespace,
	Type* type,
	uint_t ptrTypeFlags = 0
	); // returns class ref or lean data ref

JNC_INLINE
Type*
getDirectRefType (
	Type* type,
	uint_t ptrTypeFlags = 0
	)
{
	return getDirectRefType (NULL, type, ptrTypeFlags);
}

//..............................................................................

bool
isDisposableType (Type* type);

bool
isSafePtrType (Type* type);

bool
isWeakPtrType (Type* type);

Type*
getWeakPtrType (Type* type);

//..............................................................................

} // namespace ct
} // namespace jnc
