// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_PropertyType.h"
#include "jnc_PropertyVerifier.h"
#include "jnc_Function.h"

namespace jnc {

//.............................................................................

enum PropertyKind
{
	PropertyKind_Undefined = 0,
	PropertyKind_Normal,
	PropertyKind_Thunk,
	PropertyKind_DataThunk,
	PropertyKind__Count
};

//.............................................................................

enum PropertyFlagKind
{
	PropertyFlagKind_Const    = 0x010000,
	PropertyFlagKind_Bindable = 0x020000,
	PropertyFlagKind_Throws   = 0x040000,
	PropertyFlagKind_AutoGet  = 0x100000,
	PropertyFlagKind_AutoSet  = 0x200000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Property: 
	public ModuleItem,
	public Namespace
{
	friend class TypeMgr;
	friend class DerivableType;
	friend class ClassType;
	friend class FunctionMgr;
	friend class Parser;

protected:
	PropertyKind m_propertyKind;

	PropertyType* m_type;

	// construction / destruction / accessors

	Function* m_preConstructor;
	Function* m_constructor;
	Function* m_defaultConstructor;
	Function* m_staticConstructor;
	Function* m_destructor;
	Function* m_staticDestructor;

	Function* m_getter;
	Function* m_setter;
	Function* m_binder;

	// member data is CStructField or CVariable
	
	ModuleItem* m_onChanged;
	ModuleItem* m_autoGetValue;

	// parent type

	NamedType* m_parentType;
	size_t m_parentClassVTableIndex;

	rtl::Array <StructField*> m_memberFieldConstructArray;
	rtl::Array <StructField*> m_memberFieldDestructArray;
	rtl::Array <Property*> m_memberPropertyConstructArray;
	rtl::Array <Property*> m_memberPropertyDestructArray;

	// vtable

	rtl::Array <Function*> m_VTable;
	Value m_VTablePtrValue;

	PropertyVerifier m_verifier;

public:
	Property ();

	PropertyKind 
	getPropertyKind ()
	{
		return m_propertyKind;
	}

	PropertyType* 
	getType ()
	{
		return m_type;
	}

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
	getStaticConstructor ()
	{
		return m_staticConstructor;
	}

	Function* 
	getDefaultConstructor ();

	Function* 
	getDestructor ()
	{
		return m_destructor;
	}

	Function* 
	getStaticDestructor ()
	{
		return m_staticDestructor;
	}

	Function* 
	getGetter ()
	{
		return m_getter;
	}

	Function* 
	getSetter ()
	{
		return m_setter;
	}

	Function* 
	getBinder ()
	{
		return m_binder;
	}

	ModuleItem*
	getOnChanged ()
	{
		return m_onChanged;
	}

	bool
	setOnChanged (ModuleItem* item);

	bool
	createOnChanged ();

	ModuleItem*
	getAutoGetValue ()
	{
		return m_autoGetValue;
	}

	bool
	setAutoGetValue (ModuleItem* item); // struct-field or variable

	bool
	createAutoGetValue (Type* type);

	NamedType* 
	getParentType ()
	{
		return m_parentType;
	}

	bool
	isMember ()
	{
		return m_storageKind >= StorageKind_Member && m_storageKind <= StorageKind_Override;
	}

	bool
	isVirtual ()
	{
		return m_storageKind >= StorageKind_Abstract && m_storageKind <= StorageKind_Override;
	}

	size_t 
	getParentClassVTableIndex ()
	{
		return m_parentClassVTableIndex;
	}

	bool
	create (PropertyType* type);

	void
	convertToMemberProperty (NamedType* parentType);

	StructField*
	createField (
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		);

	StructField*
	createField (
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0
		)
	{
		return createField (rtl::String (), type, bitCount, ptrTypeFlags);
	}

	bool
	addMethod (Function* function);

	bool
	addProperty (Property* prop);

	bool
	callMemberFieldConstructors (const Value& thisValue);

	bool
	callMemberPropertyConstructors (const Value& thisValue);

	bool
	callMemberDestructors (const Value& thisValue);

	Value
	getVTablePtrValue ()
	{
		return m_VTablePtrValue;
	}

	virtual 
	bool
	compile ();

protected:
	virtual
	bool
	calcLayout ();

	void
	createVTablePtr ();

	Value
	getAutoAccessorPropertyValue ();

	bool 
	compileAutoGetter ();

	bool 
	compileAutoSetter ();

	bool 
	compileBinder ();

	bool
	callMemberFieldDestructors (const Value& thisValue);

	bool
	callMemberPropertyDestructors (const Value& thisValue);
};

//.............................................................................

} // namespace jnc {
