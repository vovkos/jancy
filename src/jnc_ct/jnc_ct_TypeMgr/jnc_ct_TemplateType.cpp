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
#include "jnc_ct_TemplateType.h"
#include "jnc_ct_DeclTypeCalc.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
TemplateDeclType::instantiate(const sl::ArrayRef<Type*>& argArray) {
	Type* baseType = m_declarator.getBaseType();
	if (baseType->getTypeKindFlags() & TypeKindFlag_Template) {
		TypeKind typeKind = baseType->getTypeKind();
		switch (typeKind) {
		case TypeKind_TemplateArg:
			baseType = argArray[((TemplateArgType*)baseType)->getIndex()];
			break;

		default:
			err::setFormatStringError("invalid template base type: %s", baseType->getTypeString().sz());
			return NULL;
		}
	}

	DeclTypeCalc typeCalc;
	typeCalc.setTemplateArgArray(argArray);
	uint_t declFlags;

	Type* type = typeCalc.calcType(
		baseType,
		&m_declarator,
		m_declarator.getPointerPrefixList(),
		m_declarator.getSuffixList(),
		NULL,
		&declFlags
	);

	if (!type)
		return NULL;

	// do something with declFlags?
	return type;
}

//..............................................................................

} // namespace ct
} // namespace jnc
