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

enum TypeSizeLimit {
	TypeSizeLimit_StoreSize      = 64,
	TypeSizeLimit_StackAllocSize = 128,
};

//..............................................................................

JNC_INLINE
PtrTypeFlag
getFirstPtrTypeFlag(uint_t flags) {
	return (PtrTypeFlag)(1 << sl::getLoBitIdx(flags));
}

const char*
getPtrTypeFlagString(PtrTypeFlag flag);

sl::String
getPtrTypeFlagString(uint_t flags);

sl::String
getPtrTypeFlagSignature(uint_t flags);

JNC_INLINE
const char*
getFirstPtrTypeFlagString(uint_t flags) {
	return getPtrTypeFlagString(getFirstPtrTypeFlag(flags));
}

uint_t
getPtrTypeFlagsFromModifiers(uint_t modifiers);

//..............................................................................

enum VariantField {
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
getInt32TypeKind(int32_t integer);

TypeKind
getInt32TypeKind_u(uint32_t integer);

TypeKind
getInt64TypeKind(int64_t integer);

TypeKind
getInt64TypeKind_u(uint64_t integer);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
TypeKind
getBigEndianIntegerTypeKind(TypeKind typeKind) {
	return !(getTypeKindFlags(typeKind) & TypeKindFlag_BigEndian) ?
		(TypeKind)(typeKind + TypeKind__EndianDelta) :
		typeKind;
}

JNC_INLINE
TypeKind
getLittleEndianIntegerTypeKind(TypeKind typeKind) {
	return (getTypeKindFlags(typeKind) & TypeKindFlag_BigEndian) ?
		(TypeKind)(typeKind - TypeKind__EndianDelta) :
		typeKind;
}

JNC_INLINE
TypeKind
getUnsignedIntegerTypeKind(TypeKind typeKind) {
	return !(getTypeKindFlags(typeKind) & TypeKindFlag_Unsigned) ?
		(TypeKind)(typeKind + 1) :
		typeKind;
}

JNC_INLINE
TypeKind
getSignedIntegerTypeKind(TypeKind typeKind) {
	return (getTypeKindFlags(typeKind) & TypeKindFlag_Unsigned) ?
		(TypeKind)(typeKind - 1) :
		typeKind;
}

JNC_INLINE
bool
isEquivalentIntegerTypeKind(
	TypeKind typeKind1,
	TypeKind typeKind2
) {
	return getSignedIntegerTypeKind(typeKind1) == getSignedIntegerTypeKind(typeKind2);
}

//..............................................................................

sl::String
getLlvmTypeString(llvm::Type* llvmType);

//..............................................................................

struct TypeStringTuple {
	sl::String m_typeString;
	sl::String m_typeStringPrefix;
	sl::String m_typeStringSuffix;
	sl::String m_doxyTypeString;
	sl::String m_doxyLinkedTextPrefix;
	sl::String m_doxyLinkedTextSuffix;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DualTypeTuple: sl::ListLink {
	Type* m_typeArray[2][2]; // alien-friend x container-const-non-const
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Type: public ModuleItem {
	friend class TypeMgr;
	friend class CdeclCallConv_gcc64;

protected:
	TypeKind m_typeKind;
	StdType m_stdType;
	size_t m_size;
	size_t m_alignment;
	sl::String m_signature;

	Variable* m_typeVariable;
	TypeStringTuple* m_typeStringTuple;
	SimplePropertyTypeTuple* m_simplePropertyTypeTuple;
	FunctionArgTuple* m_functionArgTuple;
	DataPtrTypeTuple* m_dataPtrTypeTuple;
	DualTypeTuple* m_dualTypeTuple;

	// codegen-only

	llvm::Type* m_llvmType;
	llvm::DIType_vn m_llvmDiType;

public:
	Type();
	~Type();

	TypeKind
	getTypeKind() {
		return m_typeKind;
	}

	uint_t
	getTypeKindFlags() {
		return jnc::getTypeKindFlags(m_typeKind);
	}

	StdType
	getStdType() {
		return m_stdType;
	}

	size_t
	getSize() {
		ASSERT(m_flags & ModuleItemFlag_LayoutReady);
		return m_size;
	}

	size_t
	getAlignment() {
		ASSERT(m_flags & ModuleItemFlag_LayoutReady);
		return m_alignment;
	}

	const sl::String&
	getSignature();

	const sl::String&
	getTypeString();

	const sl::String&
	getTypeStringPrefix();

	const sl::String&
	getTypeStringSuffix();

	const sl::String&
	getDoxyTypeString();

	const sl::String&
	getDoxyLinkedTextPrefix();

	const sl::String&
	getDoxyLinkedTextSuffix();

	sl::String
	getLlvmTypeString() {
		return ct::getLlvmTypeString(getLlvmType());
	}

	llvm::Type*
	getLlvmType();

	llvm::DIType_vn
	getLlvmDiType();

	bool
	hasTypeVariable() {
		return m_typeVariable != NULL;
	}

	Variable*
	getTypeVariable();

	Value
	getUndefValue();

	Value
	getZeroValue();

	Value
	getErrorCodeValue();

	int
	cmp(Type* type) {
		return type != this ? getSignature().cmp(type->getSignature()) : 0;
	}

	bool
	ensureLayout() {
		return (m_flags & ModuleItemFlag_LayoutReady) ? true : prepareLayout();
	}

	bool
	ensureNoImports() {
		return (m_flags & (TypeFlag_NoImports | ModuleItemFlag_LayoutReady)) ? true : prepareImports();
	}

	ArrayType*
	getArrayType(size_t elementCount);

	DataPtrType*
	getDataPtrType(
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
	);

	DataPtrType*
	getDataPtrType(
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getDataPtrType(TypeKind_DataPtr, ptrTypeKind, flags);
	}

	DataPtrType*
	getDataPtrType_c(
		TypeKind typeKind = TypeKind_DataPtr,
		uint_t flags = 0
	) {
		return getDataPtrType(typeKind, DataPtrTypeKind_Thin, flags);
	}

	FunctionArg*
	getSimpleFunctionArg(uint_t ptrTypeFlags = 0);

	Type*
	foldDualType(
		bool isAlien,
		bool isContainerConst
	);

	virtual
	sl::String
	getValueString(
		const void* p,
		const char* formatSpec = NULL
	);

	virtual
	bool
	require() {
		return ensureLayout();
	}

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

protected:
	TypeStringTuple*
	getTypeStringTuple();

	bool
	prepareImports();

	bool
	prepareLayout();

	virtual
	void
	prepareSignature();

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		ASSERT(m_typeKind < TypeKind__PrimitiveTypeCount);
		prepareSimpleTypeVariable(StdType_Type);
	}

	virtual
	bool
	calcLayout() {
		ASSERT(false); // shouldn't be called unless required
		return true;
	}

	virtual
	bool
	resolveImports() {
		ASSERT(false); // shouldn't be called unless required
		return true;
	}

	virtual
	Type*
	calcFoldedDualType(
		bool isAlien,
		bool isContainerConst
	) {
		ASSERT(false);
		return this;
	}

	void
	prepareSimpleTypeVariable(StdType stdType);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TypeStringTuple*
Type::getTypeStringTuple() {
	if (!m_typeStringTuple)
		m_typeStringTuple = new TypeStringTuple;

	return m_typeStringTuple;
}

inline
const sl::String&
Type::getSignature() {
	if (m_signature.isEmpty())
		prepareSignature();

	return m_signature;
}

inline
llvm::Type*
Type::getLlvmType() {
	if (!m_llvmType)
		prepareLlvmType();

	return m_llvmType;
}

inline
llvm::DIType_vn
Type::getLlvmDiType() {
	if (!m_llvmDiType && m_typeKind)
		prepareLlvmDiType();

	return m_llvmDiType;
}

inline
Variable*
Type::getTypeVariable() {
	if (!m_typeVariable)
		prepareTypeVariable();

	return m_typeVariable;
}

//..............................................................................

class NamedType:
	public Type,
	public Namespace {
	friend class Parser;

public:
	NamedType() {
		m_namespaceKind = NamespaceKind_Type;
	}

protected:
	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = getQualifiedName();
	}

	virtual
	void
	prepareDoxyLinkedText();
};

//..............................................................................

class Typedef:
	public ModuleItem,
	public ModuleItemDecl {
	friend class TypeMgr;

protected:
	Type* m_type;
	TypedefShadowType* m_shadowType;

public:
	Typedef();

	Type*
	getType() {
		return m_type;
	}

	TypedefShadowType*
	getShadowType();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TypedefShadowType:
	public Type,
	public ModuleItemDecl {
	friend class TypeMgr;

protected:
	Typedef* m_typedef;

public:
	TypedefShadowType() {
		m_typeKind = TypeKind_TypedefShadow;
		m_typedef = NULL;
	}

	Typedef*
	getTypedef() {
		return m_typedef;
	}

	Type*
	getActualType() {
		return m_typedef->getType();
	}

protected:
	virtual
	void
	prepareSignature() {
		m_signature = "T" + m_typedef->getQualifiedName();
	}

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = getQualifiedName();
	}

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareLlvmType() {
		m_llvmType = m_typedef->getType()->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType() {
		m_llvmDiType = m_typedef->getType()->getLlvmDiType();
	}

	virtual
	bool
	resolveImports() {
		return m_typedef->getType()->ensureNoImports();
	}

	virtual
	bool
	calcLayout();
};

//..............................................................................

Type*
getSimpleType(
	TypeKind typeKind,
	Module* module
);

Type*
getSimpleType(
	StdType stdType,
	Module* module
);

Type*
getDirectRefType(
	Type* type,
	uint_t ptrTypeFlags = 0
); // returns class ref or lean data ref

//..............................................................................

inline
bool
isDualType(Type* type) {
	return (type->getFlags() & PtrTypeFlag__Dual) != 0;
}

bool
isDisposableType(Type* type);

bool
isWeakPtrType(Type* type);

bool
isSafePtrType(Type* type);

bool
isWeakPtrType(Type* type);

Type*
getWeakPtrType(Type* type);

//..............................................................................

} // namespace ct
} // namespace jnc
