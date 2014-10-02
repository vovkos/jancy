#include "pch.h"
#include "jnc_EnumType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

EnumType::EnumType ()
{
	m_typeKind = TypeKind_Enum;
	m_enumTypeKind = EnumTypeKind_Normal;
	m_flags = TypeFlagKind_Pod;
	m_baseType = NULL;
	m_baseType_i = NULL;
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

	if (!(m_baseType->getTypeKindFlags () & TypeKindFlagKind_Integer))
	{
		err::setFormatStringError ("enum base type must be integer type");
		return NULL;
	}

	m_size = m_baseType->getSize ();
	m_alignFactor = m_baseType->getAlignFactor ();

	// assign values to consts

	m_module->m_namespaceMgr.openNamespace (this);
	Unit* unit = m_itemDecl->getParentUnit ();

	if (m_enumTypeKind == EnumTypeKind_Flag)
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
		}
	}

	m_module->m_namespaceMgr.closeNamespace ();

	return true;
}

//.............................................................................

} // namespace jnc {
