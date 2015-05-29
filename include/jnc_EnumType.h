// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"

namespace jnc {

class EnumType;

//.............................................................................

enum EnumTypeFlag
{
	EnumTypeFlag_Exposed = 0x010000,
	EnumTypeFlag_BitFlag = 0x020000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
EnumTypeFlag
getFirstEnumTypeFlag (uint_t flags)
{
	return (EnumTypeFlag) (1 << rtl::getLoBitIdx (flags));
}

const char*
getEnumTypeFlagString (EnumTypeFlag flag);

rtl::String
getEnumTypeFlagString (uint_t flags);

//.............................................................................

enum EnumConstFlag
{
	EnumConstFlag_ValueReady = 0x010000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
	Type* m_baseType;
	ImportType* m_baseType_i;
	rtl::StdList <EnumConst> m_constList;

public:
	EnumType ();

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
	prepareTypeString ();

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
isBitFlagEnumType (Type* type)
{
	return 
		type->getTypeKind () == TypeKind_Enum &&
		(((EnumType*) type)->getFlags () & EnumTypeFlag_BitFlag);
}

//.............................................................................

} // namespace jnc {
