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

#include "pch.h"
#include "jnc_ct_PropertyTemplate.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
PropertyTemplate::addMethod(
	FunctionKind functionKind,
	FunctionType* functionType
) {
	bool result;

	if (functionKind != FunctionKind_Getter && functionKind != FunctionKind_Setter)
		return err::fail("property templates can only have accessors");

	if (functionKind == FunctionKind_Getter) {
		result = m_verifier.checkGetter(functionType);
		if (!result)
			return false;

		if (m_getterType)
			return err::fail("property template already has a getter");

		m_getterType = functionType;
	} else {
		result =
			m_verifier.checkSetter(functionType) &&
			m_setterType.addOverload(functionType) != -1;

		if (!result)
			return false;
	}

	return true;
}

PropertyType*
PropertyTemplate::calcType() {
	if (!m_getterType)
		return err::fail<PropertyType*>(NULL, "incomplete property: no 'get' method or 'autoget' field");

	return m_module->m_typeMgr.getPropertyType(m_getterType, m_setterType, m_typeFlags);
}

//..............................................................................

} // namespace ct
} // namespace jnc
