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
#include "jnc_ct_CallConvKind.h"

namespace jnc {
namespace ct {

class FunctionPtrType;
class NamedType;
class ClassType;
class ClassPtrType;
class ReactorClassType;
class CallConv;
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
	getCallConv() const {
		return m_callConv;
	}

	Type*
	getReturnType() const {
		return m_returnType;
	}

	Type*
	getAsyncReturnType() const {
		ASSERT(m_flags & FunctionTypeFlag_Async);
		return m_asyncReturnType;
	}

	const sl::Array<FunctionArg*>&
	getArgArray() const {
		return m_argArray;
	}

	const sl::Array<uint_t>&
	getArgFlagArray() const {
		return m_argFlagArray;
	}

	const sl::StringRef&
	getArgSignature() const;

	sl::StringRef
	getTypeModifierString() const;

	bool
	isMemberMethodType() const {
		return !m_argArray.isEmpty() && m_argArray[0]->getStorageKind() == StorageKind_This;
	}

	FunctionArg*
	getThisArg() const {
		return isMemberMethodType() ? m_argArray[0] : NULL;
	}

	Type*
	getThisArgType() const {
		return isMemberMethodType() ? m_argArray[0]->getType() : NULL;
	}

	DerivableType*
	getThisTargetType() const;

	FunctionType*
	getShortType() const {
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

	template <typename T>
	static
	uint_t
	createSignature(
		sl::String* signature,
		sl::StringRef* argSignature,
		CallConvKind callConvKind,
		Type* returnType,
		T* const* argArray,
		size_t argCount,
		uint_t flags
	);

	template <typename T>
	static
	uint_t
	createSignature(
		sl::String* signature,
		sl::StringRef* argSignature,
		CallConvKind callConvKind,
		Type* returnType,
		const sl::ArrayRef<T>& argArray,
		uint_t flags
	) {
		return createSignature(
			signature,
			argSignature,
			callConvKind,
			returnType,
			argArray.cp(),
			argArray.getCount(),
			flags
		);
	}

	static
	void
	appendFlagSignature(
		sl::String* string,
		uint_t flags
	);

	void
	appendDoxyArgString(sl::String* string) const;

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
	sl::StringRef
	createItemString(size_t index);

	template <bool IsDoxyLinkedText>
	sl::String
	createArgString();

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
FunctionType::getArgSignature() const {
	if (m_flags & TypeFlag_SignatureFinal)
		return m_argSignature;

	if (!(m_flags & TypeFlag_SignatureReady) || (m_flags & TypeFlag_LayoutReady))
		((FunctionType*)this)->prepareSignature(); // could be called multiple times

	return m_argSignature;
}

inline
uint_t
appendFunctionArgSignature(
	sl::String* string,
	Type* type
) {
	*string += type->getSignature();
	return type->getFlags();
}

inline
uint_t
appendFunctionArgSignature(
	sl::String* string,
	FunctionArg* arg
) {
	if (arg->getStorageKind() == StorageKind_This)
		*string += '&';

	return appendFunctionArgSignature(string, arg->getType());
}

template <typename T>
uint_t
FunctionType::createSignature(
	sl::String* string,
	sl::StringRef* argSignature,
	CallConvKind callConvKind,
	Type* returnType,
	T* const* argArray,
	size_t argCount,
	uint_t flags
) {
	*string = 'F';
	appendFlagSignature(string, flags);
	*string += getCallConvSignature(callConvKind);
	*string += returnType->getSignature();

	size_t length = string->getLength();
	uint_t signatureFlags = TypeFlag_SignatureFinal;
	*string += '(';

	for (size_t i = 0; i < argCount; i++)
		signatureFlags &= appendFunctionArgSignature(string, argArray[i]);

	if (flags & FunctionTypeFlag_VarArg)
		*string += '.';

	*string += ')';
	*argSignature = string->getSubString(length);
	return (signatureFlags & returnType->getFlags()) | TypeFlag_SignatureReady;
}

//..............................................................................

} // namespace ct
} // namespace jnc
