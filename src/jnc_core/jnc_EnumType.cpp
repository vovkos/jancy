#include "pch.h"
#include "jnc_EnumType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

const char*
getEnumTypeFlagString (EnumTypeFlag flag)
{
	static const char* stringTable [] =
	{
		"exposed",   // EnumTypeFlag_Exposed = 0x0010000
		"bitflag",   // EnumTypeFlag_BitFlag = 0x0020000
	};

	size_t i = rtl::getLoBitIdx32 (flag >> 12);

	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-enum-type-flag";
}

rtl::String
getEnumTypeFlagString (uint_t flags)
{
	rtl::String string;

	if (flags & EnumTypeFlag_Exposed)
		string = "exposed ";

	if (flags & EnumTypeFlag_BitFlag)
		string += "bitflag ";

	if (!string.isEmpty ())
		string.reduceLength (1);

	return string;
}

//.............................................................................

EnumType::EnumType ()
{
	m_typeKind = TypeKind_Enum;
	m_flags = TypeFlag_Pod;
	m_baseType = NULL;
	m_baseType_i = NULL;
}

void
EnumType::prepareTypeString ()
{
	m_typeString = getEnumTypeFlagString (m_flags);

	if (!m_typeString.isEmpty ())
		m_typeString += ' ';

	m_typeString.appendFormat ("enum %s", m_tag.cc ());
}

EnumConst*
EnumType::createConst (
	const rtl::String& name,
	rtl::BoxList <Token>* initializer
	)
{
	EnumConst* enumConst = AXL_MEM_NEW (EnumConst);
	enumConst->m_name = name;
	enumConst->m_parentEnumType = this;

	if (initializer)
		enumConst->m_initializer.takeOver (initializer);

	m_constList.insertTail (enumConst);

	bool result = addItem (enumConst);
	if (!result)
		return NULL;

	return enumConst;
}

bool
EnumType::calcLayout ()
{
	bool result;

	if (m_baseType_i)
		m_baseType = m_baseType_i->getActualType ();

	if (!(m_baseType->getTypeKindFlags () & TypeKindFlag_Integer))
	{
		err::setFormatStringError ("enum base type must be integer type");
		return NULL;
	}

	m_size = m_baseType->getSize ();
	m_alignment = m_baseType->getAlignment ();

	// assign values to consts

	m_module->m_namespaceMgr.openNamespace (this);
	Unit* unit = m_itemDecl->getParentUnit ();

	if (m_flags & EnumTypeFlag_BitFlag)
	{
		intptr_t value = 1;

		rtl::Iterator <EnumConst> constIt = m_constList.getHead ();
		for (; constIt; constIt++)
		{
			if (!constIt->m_initializer.isEmpty ())
			{
				result = m_module->m_operatorMgr.parseConstIntegerExpression (
					unit,
					constIt->m_initializer,
					&value
					);

				if (!result)
					return false;
			}

			constIt->m_value = value;
			constIt->m_flags |= EnumConstFlag_ValueReady;

			uint8_t hiBitIdx = rtl::getHiBitIdx (value);
			value = hiBitIdx != -1 ? 2 << hiBitIdx : 1;
		}
	}
	else
	{
		intptr_t value = 0;

		rtl::Iterator <EnumConst> constIt = m_constList.getHead ();
		for (; constIt; constIt++, value++)
		{
			if (!constIt->m_initializer.isEmpty ())
			{
				result = m_module->m_operatorMgr.parseConstIntegerExpression (
					unit,
					constIt->m_initializer,
					&value
					);

				if (!result)
					return false;
			}

			constIt->m_value = value;
			constIt->m_flags |= EnumConstFlag_ValueReady;
		}
	}

	m_module->m_namespaceMgr.closeNamespace ();

	return true;
}

//.............................................................................

} // namespace jnc {
