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
#include "jnc_EnumType.h"

namespace jnc {
namespace ct {

class EnumType;

//..............................................................................

JNC_INLINE
EnumTypeFlag
getFirstEnumTypeFlag(uint_t flags)
{
	return (EnumTypeFlag)(1 << sl::getLoBitIdx(flags));
}

const char*
getEnumTypeFlagString(EnumTypeFlag flag);

sl::String
getEnumTypeFlagString(uint_t flags);

//..............................................................................

enum EnumConstFlag
{
	EnumConstFlag_ValueReady = 0x010000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class EnumConst:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
	friend class EnumType;
	friend class Namespace;

protected:
	EnumType* m_parentEnumType;
	int64_t m_value;

public:
	EnumConst()
	{
		m_itemKind = ModuleItemKind_EnumConst;
		m_parentEnumType = NULL;
		m_value = 0;
	}

	EnumType*
	getParentEnumType()
	{
		return m_parentEnumType;
	}

	int64_t
	getValue()
	{
		return m_value;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class EnumType: public NamedType
{
	friend class TypeMgr;
	friend class Parser;

protected:
	Type* m_baseType;
	sl::List<EnumConst> m_constList;
	sl::Array<EnumConst*> m_constArray;

public:
	EnumType();

	Type*
	getBaseType()
	{
		return m_baseType;
	}

	sl::Array<EnumConst*>
	getConstArray()
	{
		return m_constArray;
	}

	EnumConst*
	createConst(
		const sl::StringRef& name,
		sl::BoxList<Token>* initializer = NULL
		);

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);

protected:
	virtual
	void
	prepareLlvmType()
	{
		m_llvmType = m_baseType->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType()
	{
		m_llvmDiType = m_baseType->getLlvmDiType();
	}

	virtual
	bool
	calcLayout();
};

//..............................................................................

JNC_INLINE
bool
isBitFlagEnumType(Type* type)
{
	return
		type->getTypeKind() == TypeKind_Enum &&
		(((EnumType*)type)->getFlags() & EnumTypeFlag_BitFlag);
}

//..............................................................................

} // namespace ct
} // namespace jnc
