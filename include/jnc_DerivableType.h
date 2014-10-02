// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_NamedType.h"
#include "jnc_ImportType.h"
#include "jnc_Function.h"

namespace jnc {

class DerivableType;
class StructType;
class UnionType;
class ClassType;
class Function;
class Property;

//.............................................................................

class BaseTypeSlot: public UserModuleItem
{
	friend class DerivableType;
	friend class StructType;
	friend class ClassType;

protected:
	DerivableType* m_type;
	ImportType* m_type_i;

	size_t m_offset;
	size_t m_VTableIndex;
	uint_t m_llvmIndex;

public:
	BaseTypeSlot ();

	uint_t
	getFlags ()
	{
		return m_flags;
	}

	DerivableType*
	getType ()
	{
		return m_type;
	}

	ImportType*
	getType_i ()
	{
		return m_type_i;
	}

	size_t
	getOffset ()
	{
		return m_offset;
	}

	size_t
	getVTableIndex ()
	{
		return m_VTableIndex;
	}

	uint_t
	getLlvmIndex ()
	{
		return m_llvmIndex;
	}
};

//.............................................................................

class BaseTypeCoord
{
	AXL_DISABLE_COPY (BaseTypeCoord)

protected:
	char m_buffer [256];

public:
	DerivableType* m_type;
	size_t m_offset;
	rtl::Array <int32_t> m_llvmIndexArray;
	size_t m_VTableIndex;

public:
	BaseTypeCoord ();
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// unfortunately, LLVM does not natively support unions
// therefore, unnamed unions on the way to a member need special handling

struct UnionCoord
{
	UnionType* m_type;
	intptr_t m_level; // signed for simplier comparisons
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class MemberCoord: public BaseTypeCoord
{
protected:
	char m_buffer [256];

public:
	rtl::Array <UnionCoord> m_unionCoordArray;

	MemberCoord ():
		m_unionCoordArray (ref::BufKind_Field, m_buffer, sizeof (m_buffer))
	{
	}
};

//.............................................................................

class DerivableType: public NamedType
{
	friend class Parser;

protected:
	// base types

	rtl::StringHashTableMap <BaseTypeSlot*> m_baseTypeMap;
	rtl::StdList <BaseTypeSlot> m_baseTypeList;
	rtl::Array <BaseTypeSlot*> m_baseTypeArray;
	rtl::Array <BaseTypeSlot*> m_importBaseTypeArray;

	// gc roots

	rtl::Array <BaseTypeSlot*> m_gcRootBaseTypeArray;
	rtl::Array <StructField*> m_gcRootMemberFieldArray;

	// members

	rtl::Array <StructField*> m_memberFieldArray;
	rtl::Array <Function*> m_memberMethodArray;
	rtl::Array <Property*> m_memberPropertyArray;
	rtl::Array <StructField*> m_importFieldArray;
	rtl::Array <StructField*> m_unnamedFieldArray;
	rtl::Array <Variable*> m_initializedStaticFieldArray;

	// construction

	Function* m_preConstructor;
	Function* m_constructor;
	Function* m_defaultConstructor;
	Function* m_staticConstructor;
	Function* m_staticDestructor;
	Variable* m_staticOnceFlagVariable; // 'once' semantics for static constructor

	// construct arrays

	rtl::Array <BaseTypeSlot*> m_baseTypeConstructArray;
	rtl::Array <StructField*> m_memberFieldConstructArray;
	rtl::Array <Property*> m_memberPropertyConstructArray;

	// overloaded operators

	rtl::Array <Function*> m_unaryOperatorTable;
	rtl::Array <Function*> m_binaryOperatorTable;
	rtl::StringHashTableMap <Function*> m_castOperatorMap;
	Function* m_callOperator;

public:
	DerivableType ();

	rtl::ConstList <BaseTypeSlot>
	getBaseTypeList ()
	{
		return m_baseTypeList;
	}

	BaseTypeSlot*
	getBaseTypeByIndex (size_t index);

	BaseTypeSlot*
	addBaseType (Type* type);

	BaseTypeSlot*
	findBaseType (Type* type)
	{
		rtl::StringHashTableMapIterator <BaseTypeSlot*> it = m_baseTypeMap.find (type->getSignature ());
		return it ? it->m_value : NULL;
	}

	bool
	findBaseTypeTraverse (
		Type* type,
		BaseTypeCoord* coord = NULL
		)
	{
		return findBaseTypeTraverseImpl (type, coord, 0);
	}

	rtl::Array <BaseTypeSlot*>
	getGcRootBaseTypeArray ()
	{
		return m_gcRootBaseTypeArray;
	}

	rtl::Array <StructField*>
	getGcRootMemberFieldArray ()
	{
		return m_gcRootMemberFieldArray;
	}

	rtl::Array <StructField*>
	getMemberFieldArray ()
	{
		return m_memberFieldArray;
	}

	rtl::Array <Function*>
	getMemberMethodArray ()
	{
		return m_memberMethodArray;
	}

	rtl::Array <Property*>
	getMemberPropertyArray ()
	{
		return m_memberPropertyArray;
	}

	rtl::Array <Variable*>
	getInitializedStaticFieldArray ()
	{
		return m_initializedStaticFieldArray;
	}

	bool
	callBaseTypeConstructors (const Value& thisValue);

	bool
	callMemberFieldConstructors (const Value& thisValue);

	bool
	callMemberPropertyConstructors (const Value& thisValue);

	bool
	initializeStaticFields ();

	Function*
	getPreConstructor ()
	{
		return m_preConstructor;
	}

	Function*
	getConstructor ()
	{
		return m_constructor;
	}

	Function*
	getDefaultConstructor ();

	Function*
	getStaticConstructor ()
	{
		return m_staticConstructor;
	}

	Function*
	getStaticDestructor ()
	{
		return m_staticDestructor;
	}

	Variable*
	getStaticOnceFlagVariable ()
	{
		return m_staticOnceFlagVariable;
	}

	Function*
	getUnaryOperator (UnOpKind opKind)
	{
		ASSERT ((size_t) opKind < UnOpKind__Count);
		return m_unaryOperatorTable ? m_unaryOperatorTable [opKind] : NULL;
	}

	Function*
	getBinaryOperator (BinOpKind opKind)
	{
		ASSERT ((size_t) opKind < BinOpKind__Count);
		return m_binaryOperatorTable ? m_binaryOperatorTable [opKind] : NULL;
	}

	Function*
	getCallOperator ()
	{
		return m_callOperator;
	}

	virtual
	StructField*
	getFieldByIndex (size_t index) = 0;

	StructField*
	createField (
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		)
	{
		return createFieldImpl (name, type, bitCount, ptrTypeFlags, constructor, initializer);
	}

	StructField*
	createField (
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0
		)
	{
		return createFieldImpl (rtl::String (), type, bitCount, ptrTypeFlags);
	}

	Function*
	createMethod (
		StorageKind storageKind,
		const rtl::String& name,
		FunctionType* shortType
		);

	Function*
	createUnnamedMethod (
		StorageKind storageKind,
		FunctionKind functionKind,
		FunctionType* shortType
		);

	Property*
	createProperty (
		StorageKind storageKind,
		const rtl::String& name,
		PropertyType* shortType
		);

	virtual
	bool
	addMethod (Function* function);

	virtual
	bool
	addProperty (Property* prop);

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
		) = 0;

	bool
	resolveImportBaseType (BaseTypeSlot* slot);

	bool
	resolveImportBaseTypes ();

	bool
	resolveImportFields ();

	bool
	createDefaultMethod (
		FunctionKind functionKind,
		StorageKind storageKind = StorageKind_Member
		);

	bool
	compileDefaultStaticConstructor ();

	bool
	compileDefaultPreConstructor ();

	bool
	compileDefaultConstructor ();

	bool
	findBaseTypeTraverseImpl (
		Type* type,
		BaseTypeCoord* coord,
		size_t level
		);

	virtual
	ModuleItem*
	findItemTraverseImpl (
		const char* name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		)
	{
		return findItemTraverseImpl (name, coord, flags, 0);
	}

	ModuleItem*
	findItemTraverseImpl (
		const char* name,
		MemberCoord* coord,
		uint_t flags,
		size_t baseTypeLevel
		);
};

//.............................................................................

} // namespace jnc {
