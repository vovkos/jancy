// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_PropertyType.h"
#include "jnc_ct_PropertyVerifier.h"
#include "jnc_ct_Function.h"
#include "jnc_ct_NamedTypeBlock.h"

namespace jnc {
namespace ct {

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

enum PropertyFlag
{
	PropertyFlag_Const    = 0x010000,
	PropertyFlag_Bindable = 0x020000,
	PropertyFlag_AutoGet  = 0x100000,
	PropertyFlag_AutoSet  = 0x200000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Property: 
	public ModuleItem,
	public Namespace,
	public NamedTypeBlock
{
	friend class TypeMgr;
	friend class NamedTypeBlock;
	friend class DerivableType;
	friend class ClassType;
	friend class FunctionMgr;
	friend class Parser;

protected:
	PropertyKind m_propertyKind;

	PropertyType* m_type;

	Function* m_getter;
	Function* m_setter;
	Function* m_binder;

	// member data is StructField or Variable
	
	ModuleItem* m_onChanged;
	ModuleItem* m_autoGetValue;

	// parent type

	DerivableType* m_parentType;
	size_t m_parentClassVTableIndex;
	sl::Array <Function*> m_vtable;
	Variable* m_vtableVariable;

	ExtensionNamespace* m_extensionNamespace;

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

	DerivableType* 
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

	virtual
	bool
	addMethod (Function* function);

	virtual
	bool
	addProperty (Property* prop);

	Variable*
	getVTableVariable ()
	{
		return m_vtableVariable;
	}

	virtual 
	bool
	compile ();

protected:
	virtual
	StructField*
	createFieldImpl (
		const sl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		);

	virtual
	bool
	calcLayout ();

	void
	createVTableVariable ();

	Value
	getAutoAccessorPropertyValue ();

	bool 
	compileAutoGetter ();

	bool 
	compileAutoSetter ();

	bool 
	compileBinder ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
