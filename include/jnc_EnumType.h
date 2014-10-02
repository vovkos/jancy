// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"
#include "jnc_NamedType.h"

namespace jnc {

class EnumType;

//.............................................................................

enum EnumTypeKind
{
	EnumTypeKind_Normal,
	EnumTypeKind_Flag,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EnumTypeFlagKind
{
	EnumTypeFlagKind_Exposed = 0x010000,
};

//.............................................................................

class EnumConst: 
	public UserModuleItem,
	public ModuleItemInitializer
{
	friend class EnumType;
	friend class Namespace;

protected:
	EnumType* m_parentEnumType;
	intptr_t m_value;

public:
	EnumConst ()
	{
		m_itemKind = ModuleItemKind_EnumConst;
		m_parentEnumType = NULL;
		m_value = 0;
	}

	EnumType*
	getParentEnumType ()
	{
		return m_parentEnumType;
	}

	intptr_t
	getValue ()
	{
		return m_value;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class EnumType: public NamedType
{
	friend class TypeMgr;
	friend class Parser;
	
protected:
	EnumTypeKind m_enumTypeKind;

	Type* m_baseType;
	ImportType* m_baseType_i;
	rtl::StdList <EnumConst> m_constList;

public:
	EnumType ();

	EnumTypeKind 
	getEnumTypeKind ()
	{
		return m_enumTypeKind;
	}

	Type*
	getBaseType ()
	{
		return m_baseType;
	}

	ImportType*
	getBaseType_i ()
	{
		return m_baseType_i;
	}

	rtl::ConstList <EnumConst>
	getConstList ()
	{
		return m_constList;
	}

	EnumConst*
	createConst (
		const rtl::String& name,
		rtl::BoxList <Token>* initializer = NULL
		);

protected:
	virtual 
	void
	prepareTypeString ()
	{
		m_typeString.format (
			(m_flags & EnumTypeFlagKind_Exposed) ? 
				"cenum %s" : 
				"enum %s", 
			m_tag.cc () // thanks a lot gcc
			);
	}

	virtual 
	void
	prepareLlvmType ()
	{
		m_llvmType = m_baseType->getLlvmType ();
	}

	virtual 
	void
	prepareLlvmDiType ()
	{
		m_llvmDiType = m_baseType->getLlvmDiType ();
	}

	virtual 
	bool
	calcLayout ();
};

//.............................................................................

inline
bool 
isFlagEnumType (Type* type)
{
	return 
		type->getTypeKind () == TypeKind_Enum &&
		((EnumType*) type)->getEnumTypeKind () == EnumTypeKind_Flag;
}

//.............................................................................

} // namespace jnc {
