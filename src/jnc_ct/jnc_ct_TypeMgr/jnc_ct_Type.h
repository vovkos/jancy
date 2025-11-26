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
class ImportType;
class FunctionArg;
class Value;

struct DataPtrTypeTuple;
struct SimplePropertyTypeTuple;
struct FunctionArgTuple;
struct TemplateInstance;

//..............................................................................

enum TypeSizeLimit {
	TypeSizeLimit_StoreSize      = 64,
	TypeSizeLimit_StackAllocSize = 128,
};

//..............................................................................

const char*
getPtrTypeFlagString(PtrTypeFlag flag);

sl::StringRef
getPtrTypeFlagString(uint_t flags);

sl::StringRef
getPtrTypeFlagSignature(uint_t flags);

uint_t
getPtrTypeFlagsFromModifiers(uint_t modifiers);

//..............................................................................

enum VariantField {
	VariantField_Data    = 0,
	VariantField_Padding = 1,
	VariantField_Type    = 2,
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
getUnsignedIntegerTypeKind(TypeKind typeKind) {
	return (getTypeKindFlags(typeKind) & TypeKindFlag_Signed) ?
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
	sl::StringRef m_signature; // not part of StringCache for performance

	Variable* m_typeVariable;
	SimplePropertyTypeTuple* m_simplePropertyTypeTuple;
	FunctionArgTuple* m_functionArgTuple;
	DataPtrTypeTuple* m_dataPtrTypeTuple;
	DualTypeTuple* m_dualTypeTuple;

	// codegen-only

	llvm::Type* m_llvmType;
	llvm::DIType_vn m_llvmDiType;

public:
	Type();

	virtual
	Type*
	getItemType() {
		return this;
	}

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
		ASSERT(m_flags & TypeFlag_LayoutReady);
		return m_size;
	}

	size_t
	getAlignment() {
		ASSERT(m_flags & TypeFlag_LayoutReady);
		return m_alignment;
	}

	const sl::StringRef&
	getSignature() const;

	const sl::StringRef&
	getTypeString() {
		return getItemString(TypeStringKind_TypeName);
	}

	const sl::StringRef&
	getTypeStringPrefix() {
		return getItemString(TypeStringKind_Prefix);
	}

	const sl::StringRef&
	getTypeStringSuffix() {
		return getItemString(TypeStringKind_Suffix);
	}

	const sl::StringRef&
	getDoxyTypeString() {
		return getItemString(TypeStringKind_DoxyTypeString);
	}

	const sl::StringRef&
	getDoxyLinkedTextPrefix() {
		return getItemString(TypeStringKind_DoxyLinkedTextPrefix);
	}

	const sl::StringRef&
	getDoxyLinkedTextSuffix() {
		return getItemString(TypeStringKind_DoxyLinkedTextSuffix);
	}

	sl::String
	getLlvmTypeString() {
		return ct::getLlvmTypeString(getLlvmType());
	}

	llvm::Type*
	getLlvmType();

	llvm::DIType_vn
	getLlvmDiType();

	bool
	isEqual(Type* type) const {
		return type == this || type->getSignature() == getSignature();
	}

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

	bool
	ensureLayout() {
		return (m_flags & TypeFlag_LayoutReady) ? true : prepareLayout();
	}

	bool
	ensureNoImports() {
		return (m_flags & (TypeFlag_NoImports | TypeFlag_LayoutReady)) ? true : prepareImports();
	}

	ArrayType*
	getArrayType(size_t elementCount);

	DataPtrType*
	getDataPtrType(
		uint_t bitOffset,
		uint_t bitCount,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
	);

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

	virtual
	sl::StringRef
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

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	) {
		return true; // scalar types deduce nothing
	}

protected:
	virtual
	void
	prepareSignature() {
		ASSERT(false); // shouldn't be called unless required
	}

	virtual
	sl::StringRef
	createItemString(size_t index);

	bool
	prepareImports();

	bool
	prepareLayout();

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

	void
	setTemplateArgDeductionError(Type* argValueType) {
		err::setFormatStringError(
			"incompatible types while deducing template argument: '%s' vs '%s'",
			argValueType->getTypeString().sz(),
			getTypeString().sz()
		);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Type::Type() {
	m_itemKind = ModuleItemKind_Type;
	m_typeKind = TypeKind_Void;
	m_stdType = (StdType)-1;
	m_size = 0;
	m_alignment = 1;
	m_llvmType = NULL;
	m_typeVariable = NULL;
	m_simplePropertyTypeTuple = NULL;
	m_functionArgTuple = NULL;
	m_dataPtrTypeTuple = NULL;
	m_dualTypeTuple = NULL;
}


inline
const sl::StringRef&
Type::getSignature() const {
	if (m_flags & TypeFlag_SignatureFinal)
		return m_signature;

	if (!(m_flags & TypeFlag_SignatureReady) || (m_flags & TypeFlag_LayoutReady))
		((Type*)this)->prepareSignature(); // could be called multiple times

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

class NamedType: public ModuleItemWithNamespace<Type> {
	friend class Parser;

protected:
	TemplateInstance* m_templateInstance;

public:
	NamedType() {
		m_namespaceKind = NamespaceKind_Type;
		m_templateInstance = NULL;
	}

	TemplateInstance*
	getTemplateInstance() {
		return m_templateInstance;
	}

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	sl::StringRef
	createLinkId();

	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	void
	prepareSignature();
};

//..............................................................................

template <
	typename T,
	typename B,
	TypeKind typeKind,
	uint16_t signaturePrefix
>
class ModType: public T {
	friend class TypeMgr;

protected:
	B* m_baseType;
	uint_t m_typeModifiers;

public:
	ModType() {
		m_typeKind = typeKind;
		m_baseType = NULL;
		m_typeModifiers = 0;
	}

	B*
	getBaseType() {
		return m_baseType;
	}

	uint_t
	getTypeModifiers() {
		return m_typeModifiers;
	}

	static
	sl::String
	createSignature(
		B* baseType,
		uint_t typeModifiers
	) {
		const sl::StringRef& baseSignature = baseType->getSignature();
		size_t baseSignatureLength = baseSignature.getLength();

		sl::String signature;
		char* p = signature.createBuffer(2 + baseSignatureLength + 10);
		*(uint16_t*)p = signaturePrefix;
		p += sizeof(uint16_t);
		memcpy(p, baseSignature.cp(), baseSignatureLength);
		p += baseSignatureLength;
		int length = sprintf(p, ":%x", typeModifiers);
		p += length;
		signature.overrideLength(p - signature.cp());
		return signature;
	}

protected:
	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_baseType, m_typeModifiers);
		m_flags |= TypeFlag_SignatureReady;
	}
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
isDisposableType(DerivableType* type);

bool
isDisposableType(Type* type);

bool
isStringableType(DerivableType* type);

bool
isStringableType(Type* type);

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
