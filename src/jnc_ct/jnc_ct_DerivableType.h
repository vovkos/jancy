// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ImportType.h"
#include "jnc_ct_Function.h"
#include "jnc_ct_NamedTypeBlock.h"

namespace jnc {
namespace ct {

class DerivableType;
class StructType;
class UnionType;
class ClassType;
class Function;
class Property;

//..............................................................................

class BaseTypeSlot:
	public ModuleItem,
	public ModuleItemDecl
{
	friend class DerivableType;
	friend class StructType;
	friend class ClassType;

protected:
	DerivableType* m_type;
	size_t m_offset;
	size_t m_vtableIndex;
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

	size_t
	getOffset ()
	{
		return m_offset;
	}

	size_t
	getVTableIndex ()
	{
		return m_vtableIndex;
	}

	uint_t
	getLlvmIndex ()
	{
		return m_llvmIndex;
	}
};

//..............................................................................

class BaseTypeCoord
{
	AXL_DISABLE_COPY (BaseTypeCoord)

protected:
	char m_buffer [256];

public:
	DerivableType* m_type;
	size_t m_offset;
	sl::Array <int32_t> m_llvmIndexArray;
	size_t m_vtableIndex;

public:
	BaseTypeCoord ();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// unfortunately, LLVM does not natively support unions
// therefore, unnamed unions on the way to a member need special handling

struct UnionCoord
{
	UnionType* m_type;
	intptr_t m_level; // signed for simplier comparisons
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class MemberCoord: public BaseTypeCoord
{
protected:
	char m_buffer [256];

public:
	sl::Array <UnionCoord> m_unionCoordArray;

	MemberCoord ():
		m_unionCoordArray (ref::BufKind_Field, m_buffer, sizeof (m_buffer))
	{
	}
};

//..............................................................................

class DerivableType:
	public NamedType,
	public NamedTypeBlock
{
	friend class Parser;

protected:
	// base types

	sl::StringHashTableMap <BaseTypeSlot*> m_baseTypeMap;
	sl::StdList <BaseTypeSlot> m_baseTypeList;
	sl::Array <BaseTypeSlot*> m_baseTypeArray;
	sl::Array <BaseTypeSlot*> m_gcRootBaseTypeArray;
	sl::Array <BaseTypeSlot*> m_baseTypeConstructArray;
	sl::Array <BaseTypeSlot*> m_baseTypeDestructArray;

	Type* m_setAsType;
	Function* m_defaultConstructor;
	sl::Array <ExtensionNamespace*> m_extensionNamespaceArray;

	// overloaded operators

	sl::Array <Function*> m_unaryOperatorTable;
	sl::Array <Function*> m_binaryOperatorTable;
	sl::Array <Function*> m_castOperatorTable;
	sl::StringHashTableMap <Function*> m_castOperatorMap;
	Function* m_callOperator;
	Function* m_operatorVararg;
	Function* m_operatorCdeclVararg;

public:
	DerivableType ();

	virtual
	Type*
	getThisArgType (uint_t ptrTypeFlags)
	{
		return (Type*) getDataPtrType (DataPtrTypeKind_Normal, ptrTypeFlags);
	}

	FunctionType*
	getMemberMethodType (
		FunctionType* shortType,
		uint_t thisArgTypeFlags = 0
		);

	PropertyType*
	getMemberPropertyType (PropertyType* shortType);

	sl::ConstList <BaseTypeSlot>
	getBaseTypeList ()
	{
		return m_baseTypeList;
	}

	sl::Array <BaseTypeSlot*>
	getBaseTypeArray ()
	{
		return m_baseTypeArray;
	}

	BaseTypeSlot*
	getBaseTypeByIndex (size_t index);

	BaseTypeSlot*
	addBaseType (Type* type);

	BaseTypeSlot*
	findBaseType (Type* type)
	{
		sl::StringHashTableMapIterator <BaseTypeSlot*> it = m_baseTypeMap.find (type->getSignature ());
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

	size_t
	findBaseTypeOffset (Type* type);

	sl::Array <BaseTypeSlot*>
	getGcRootBaseTypeArray ()
	{
		return m_gcRootBaseTypeArray;
	}

	Type*
	getSetAsType ()
	{
		return m_setAsType;
	}

	Function*
	getPreConstructor ()
	{
		return m_preconstructor;
	}

	Function*
	getDefaultConstructor ();

	Function*
	getUnaryOperator (UnOpKind opKind)
	{
		return (size_t) opKind < m_unaryOperatorTable.getCount () ? m_unaryOperatorTable [opKind] : NULL;
	}

	Function*
	getBinaryOperator (BinOpKind opKind)
	{
		return (size_t) opKind < m_binaryOperatorTable.getCount () ? m_binaryOperatorTable [opKind] : NULL;
	}

	Function*
	getCastOperator (size_t i)
	{
		return i < m_castOperatorTable.getCount () ? m_castOperatorTable [i] : NULL;
	}

	Function*
	getCastOperator (Type* type)
	{
		sl::StringHashTableMapIterator <Function*> it = m_castOperatorMap.find (type->getSignature ());
		return it ? it->m_value : NULL;
	}

	Function*
	getCallOperator ()
	{
		return m_callOperator;
	}

	Function*
	getOperatorVararg ()
	{
		return m_operatorVararg;
	}

	Function*
	getOperatorCdeclVararg ()
	{
		return m_operatorCdeclVararg;
	}

	StructField*
	getFieldByIndex (size_t index);

	virtual
	bool
	addMethod (Function* function);

	virtual
	bool
	addProperty (Property* prop);

	bool
	callBaseTypeConstructors (const Value& thisValue);

	bool
	callBaseTypeDestructors (const Value& thisValue);

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);

protected:
	ModuleItem*
	findItemInExtensionNamespaces (const sl::StringRef& name);

	bool
	createDefaultMethod (
		FunctionKind functionKind,
		StorageKind storageKind = StorageKind_Member
		);

	bool
	compileDefaultStaticConstructor ();

	bool
	compileDefaultConstructor ();

	bool
	compileDefaultDestructor ();

	bool
	findBaseTypeTraverseImpl (
		Type* type,
		BaseTypeCoord* coord,
		size_t level
		);

	virtual
	ModuleItem*
	findItemTraverseImpl (
		const sl::StringRef& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		)
	{
		return findItemTraverseImpl (name, coord, flags, 0);
	}

	ModuleItem*
	findItemTraverseImpl (
		const sl::StringRef& name,
		MemberCoord* coord,
		uint_t flags,
		size_t baseTypeLevel
		);
};

//..............................................................................

JNC_INLINE
bool
isConstructibleType (Type* type)
{
	return
		(type->getTypeKindFlags () & TypeKindFlag_Derivable) &&
		((DerivableType*) type)->getConstructor () != NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
