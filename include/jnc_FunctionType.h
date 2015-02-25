// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CallConv.h"
#include "jnc_FunctionArg.h"

namespace jnc {

class FunctionPtrType;
class NamedType;
class ClassType;
class ClassPtrType;
class ReactorClassType;
class CdeclCallConv_msc64;
class Function;

struct FunctionPtrTypeTuple;

//.............................................................................

enum FunctionTypeFlag
{
	FunctionTypeFlag_VarArg      = 0x010000,
	FunctionTypeFlag_Throws      = 0x020000,
	FunctionTypeFlag_CoercedArgs = 0x040000,
	FunctionTypeFlag_Automaton   = 0x080000,
};

//.............................................................................

enum FunctionPtrTypeKind
{
	FunctionPtrTypeKind_Normal = 0,
	FunctionPtrTypeKind_Weak,
	FunctionPtrTypeKind_Thin,
	FunctionPtrTypeKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getFunctionPtrTypeKindString (FunctionPtrTypeKind ptrTypeKind);

//.............................................................................

class FunctionType: public Type
{
	friend class TypeMgr;
	friend class ClassType;
	friend class CdeclCallConv_msc64;
	friend class CdeclCallConv_gcc64;

protected:
	CallConv* m_callConv;
	Type* m_returnType;
	ImportType* m_returnType_i;
	rtl::Array <FunctionArg*> m_argArray;
	rtl::String m_argSignature;
	rtl::String m_typeModifierString;
	rtl::String m_argString;
	FunctionType* m_shortType;
	FunctionType* m_stdObjectMemberMethodType;
	Function* m_abstractFunction;
	FunctionPtrTypeTuple* m_functionPtrTypeTuple;
	ReactorClassType* m_reactorInterfaceType;

public:
	FunctionType ();

	CallConv*
	getCallConv ()
	{
		return m_callConv;
	}

	Type*
	getReturnType ()
	{
		return m_returnType;
	}

	ImportType*
	getReturnType_i ()
	{
		return m_returnType_i;
	}

	rtl::Array <FunctionArg*>
	getArgArray ()
	{
		return m_argArray;
	}

	rtl::String
	getArgSignature ();

	rtl::String
	getArgString ();

	rtl::String
	getTypeModifierString ();

	bool
	isMemberMethodType ()
	{
		return !m_argArray.isEmpty () && m_argArray [0]->getStorageKind () == StorageKind_This;
	}

	Type*
	getThisArgType ()
	{
		return isMemberMethodType () ? m_argArray [0]->getType () : NULL;
	}

	DerivableType*
	getThisTargetType ();

	FunctionType*
	getShortType ()
	{
		return m_shortType;
	}

	FunctionType*
	getMemberMethodType (
		DerivableType* type,
		uint_t thisArgFlags = 0
		);

	FunctionType*
	getStdObjectMemberMethodType ();

	Function*
	getAbstractFunction ();

	FunctionPtrType*
	getFunctionPtrType (
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
		);

	FunctionPtrType*
	getFunctionPtrType (
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getFunctionPtrType (TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	ClassType*
	getMulticastType ();

	static
	rtl::String
	createSignature (
		CallConv* callConv,
		Type* returnType,
		Type* const* argTypeArray,
		size_t argCount,
		uint_t flags
		);

	static
	rtl::String
	createSignature (
		CallConv* callConv,
		Type* returnType,
		FunctionArg* const* argArray,
		size_t argCount,
		uint_t flags
		);

	static
	rtl::String
	createArgSignature (
		Type* const* argTypeArray,
		size_t argCount,
		uint_t flags
		);

	static
	rtl::String
	createArgSignature (
		FunctionArg* const* argArray,
		size_t argCount,
		uint_t flags
		);

	rtl::String
	createArgSignature ()
	{
		return createArgSignature (m_argArray, m_argArray.getCount (), m_flags);
	}

	virtual
	bool
	compile ();

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();

	virtual
	bool
	calcLayout ();
};

//.............................................................................

} // namespace jnc {
