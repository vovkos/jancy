#include "pch.h"
#include "jnc_PropertyTemplate.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

PropertyTemplate::PropertyTemplate ()
{
	m_itemKind = ModuleItemKind_PropertyTemplate;
	m_namespaceKind = NamespaceKind_PropertyTemplate;
	m_itemDecl = this;
	m_getterType = NULL;
	m_typeFlags = 0;
}

bool
PropertyTemplate::addMethod (
	FunctionKind functionKind,
	FunctionType* functionType
	)
{
	bool result;

	if (functionKind != FunctionKind_Getter && functionKind != FunctionKind_Setter)
	{
		err::setFormatStringError ("property templates can only have accessors");
		return false;
	}

	if (functionKind == FunctionKind_Getter)
	{
		result = m_verifier.checkGetter (functionType);
		if (!result)
			return false;

		if (m_getterType)
		{
			err::setFormatStringError ("property template already has a getter");
			return false;
		}

		m_getterType = functionType;
	}
	else
	{
		result = 
			m_verifier.checkSetter (functionType) &&
			m_setterType.addOverload (functionType);

		if (!result)
			return false;
	}

	return true;
}

PropertyType*
PropertyTemplate::calcType ()
{
	if (!m_getterType)
	{
		err::setFormatStringError ("incomplete property template: no 'get' method");
		return NULL;
	}

	return m_module->m_typeMgr.getPropertyType (m_getterType, m_setterType, m_typeFlags);
}

//.............................................................................

} // namespace jnc {
