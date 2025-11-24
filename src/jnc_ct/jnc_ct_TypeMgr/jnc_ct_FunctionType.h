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

#include "jnc_FunctionType.h"
#include "jnc_ct_Type.h"
#include "jnc_ct_FunctionArg.h"

namespace jnc {
namespace ct {

class CallConv;
class FunctionPtrType;
class NamedType;
class ClassType;
class ClassPtrType;
class ReactorClassType;
class CdeclCallConv_msc64;
class Function;

struct FunctionPtrTypeTuple;

//..............................................................................

class FunctionType: public Type {
	friend class TypeMgr;
	friend class ClassType;
	friend class CallConv;
	friend class CallConv_msc32;
	friend class CallConv_gcc32;
	friend class CdeclCallConv_msc64;
	friend class CdeclCallConv_gcc64;
	friend class CdeclCallConv_arm;

protected:
	CallConv* m_callConv;
	Type* m_returnType;
	Type* m_asyncReturnType; // until we have generics (e.g. Promise<T>)
	sl::Array<FunctionArg*> m_argArray;
	sl::Array<uint_t> m_argFlagArray; // args can be shared between func types
	sl::StringRef m_argSignature;
	FunctionType* m_shortType;
	FunctionType* m_stdObjectMemberMethodType;
	FunctionPtrTypeTuple* m_functionPtrTypeTuple;

public:
	FunctionType();

	CallConv*
	getCallConv() {
		return m_callConv;
	}

	Type*
	getReturnType() {
		return m_returnType;
	}

	Type*
	getAsyncReturnType() {
		ASSERT(m_flags & FunctionTypeFlag_Async);
		return m_asyncReturnType;
	}

	const sl::Array<FunctionArg*>&
	getArgArray() {
		return m_argArray;
	}

	const sl::Array<uint_t>&
	getArgFlagArray() {
		return m_argFlagArray;
	}

	const sl::StringRef&
	getArgSignature();

	sl::StringRef
	getTypeModifierString();

	bool
	isMemberMethodType() {
		return !m_argArray.isEmpty() && m_argArray[0]->getStorageKind() == StorageKind_This;
	}

	FunctionArg*
	getThisArg() {
		return isMemberMethodType() ? m_argArray[0] : NULL;
	}

	Type*
	getThisArgType() {
		return isMemberMethodType() ? m_argArray[0]->getType() : NULL;
	}

	DerivableType*
	getThisTargetType();

	FunctionType*
	getShortType() {
		return m_shortType;
	}

	FunctionType*
	getMemberMethodType(
		DerivableType* type,
		uint_t thisArgFlags = 0
	);

	FunctionType*
	getStdObjectMemberMethodType();

	FunctionPtrType*
	getFunctionPtrType(
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
	);

	FunctionPtrType*
	getFunctionPtrType(
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getFunctionPtrType(TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	ClassType*
	getMulticastType();

	// signature functions return TypeFlag_SignatureFinal or 0

	static
	uint_t
	createSignature(
		sl::String* signature,
		sl::StringRef* argSignature,
		CallConv* callConv,
		Type* returnType,
		Type* const* argTypeArray,
		size_t argCount,
		uint_t flags
	);

	static
	uint_t
	createSignature(
		sl::String* signature,
		sl::StringRef* argSignature,
		CallConv* callConv,
		Type* returnType,
		FunctionArg* const* argArray,
		size_t argCount,
		uint_t flags
	);

	static
	void
	appendFlagSignature(
		sl::String* string,
		uint_t flags
	);

	static
	uint_t
	appendArgSignature(
		sl::String* string,
		Type* const* argTypeArray,
		size_t argCount,
		uint_t flags
	);

	static
	uint_t
	appendArgSignature(
		sl::String* string,
		FunctionArg* const* argArray,
		size_t argCount,
		uint_t flags
	);

	uint_t
	appendArgSignature(sl::String* string) {
		return appendArgSignature(string, m_argArray, m_argArray.getCount(), m_flags);
	}

	void
	appendDoxyArgString(sl::String* string);

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	bool
	resolveImports();

	virtual
	bool
	calcLayout();

	virtual
	void
	prepareSignature();

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareDoxyTypeString();

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_FunctionType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
FunctionType::FunctionType() {
	m_typeKind = TypeKind_Function;
	m_callConv = NULL;
	m_returnType = NULL;
	m_asyncReturnType = NULL;
	m_shortType = this;
	m_stdObjectMemberMethodType = NULL;
	m_functionPtrTypeTuple = NULL;
	m_functionPtrTypeTuple = NULL;
}

inline
const sl::StringRef&
FunctionType::getArgSignature() {
	if (!(m_flags & TypeFlag_SignatureFinal))
		prepareSignature();

	return m_argSignature;
}

//..............................................................................

} // namespace ct
} // namespace jnc
