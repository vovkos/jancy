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
#include "jnc_ct_FunctionArg.h"
#include "jnc_FunctionType.h"

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

class FunctionType: public Type
{
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
	sl::String m_argSignature;
	FunctionType* m_shortType;
	FunctionType* m_stdObjectMemberMethodType;
	Function* m_abstractFunction;
	FunctionPtrTypeTuple* m_functionPtrTypeTuple;

public:
	FunctionType();

	CallConv*
	getCallConv()
	{
		return m_callConv;
	}

	Type*
	getReturnType()
	{
		return m_returnType;
	}

	Type*
	getAsyncReturnType()
	{
		ASSERT(m_flags & FunctionTypeFlag_Async);
		return m_asyncReturnType;
	}

	sl::Array<FunctionArg*>
	getArgArray()
	{
		return m_argArray;
	}

	sl::Array<uint_t>
	getArgFlagArray()
	{
		return m_argFlagArray;
	}

	const sl::String&
	getArgSignature();

	sl::String
	getTypeModifierString();

	bool
	isMemberMethodType()
	{
		return !m_argArray.isEmpty() && m_argArray[0]->getStorageKind() == StorageKind_This;
	}

	FunctionArg*
	getThisArg()
	{
		return isMemberMethodType() ? m_argArray[0] : NULL;
	}

	Type*
	getThisArgType()
	{
		return isMemberMethodType() ? m_argArray[0]->getType() : NULL;
	}

	DerivableType*
	getThisTargetType();

	FunctionType*
	getShortType()
	{
		return m_shortType;
	}

	FunctionType*
	getMemberMethodType(
		DerivableType* type,
		uint_t thisArgFlags = 0
		);

	FunctionType*
	getStdObjectMemberMethodType();

	Function*
	getAbstractFunction();

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
		)
	{
		return getFunctionPtrType(TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	ClassType*
	getMulticastType();

	static
	sl::String
	createSignature(
		CallConv* callConv,
		Type* returnType,
		Type* const* argTypeArray,
		size_t argCount,
		uint_t flags
		);

	static
	sl::String
	createSignature(
		CallConv* callConv,
		Type* returnType,
		FunctionArg* const* argArray,
		size_t argCount,
		uint_t flags
		);

	static
	sl::String
	createFlagSignature(uint_t flags);

	static
	sl::String
	createArgSignature(
		Type* const* argTypeArray,
		size_t argCount,
		uint_t flags
		);

	static
	sl::String
	createArgSignature(
		FunctionArg* const* argArray,
		size_t argCount,
		uint_t flags
		);

	sl::String
	createArgSignature()
	{
		return createArgSignature(m_argArray, m_argArray.getCount(), m_flags);
	}

	virtual
	bool
	compile();

	sl::String
	getDoxyArgString();

protected:
	virtual
	void
	prepareSignature()
	{
		m_signature = createSignature(m_callConv, m_returnType, m_argArray, m_argArray.getCount(), m_flags);
	}

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
};

//..............................................................................

} // namespace ct
} // namespace jnc
