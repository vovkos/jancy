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
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
TemplateArgType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	sl::Array<Type*>::Rwi rwi = templateArgTypeArray->rwi();
	if (!rwi[m_index])
		rwi[m_index] = referenceType;
	else {
		Type* actualType = selectTemplateArg(rwi[m_index], referenceType);
		if (!actualType)
			return false;

		rwi[m_index] = actualType;
	}

	return true;
}

Type*
TemplateArgType::selectTemplateArg(
	Type* type1,
	Type* type2
) {
	if (type1->isEqual(type2))
		return type1;

	Type* type = getArithmeticOperatorResultType(type1, type2);
	if (type)
		return type;

	err::setFormatStringError(
		"conflict deducing argument '%s': '%s' vs '%s'",
		m_name.sz(),
		type1->getTypeString().sz(),
		type2->getTypeString().sz()
	);

	return NULL;
}

//..............................................................................

bool
TemplatePtrType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	TypeKind typeKind = referenceType->getTypeKind();
	switch (typeKind) {
	case TypeKind_DataRef:
		if (((DataPtrType*)referenceType)->getTargetType()->getTypeKind() != TypeKind_Array)
			break;

		referenceType = m_module->m_operatorMgr.prepareArrayRefType((DataPtrType*)referenceType);
		// and fall through

	case TypeKind_DataPtr:
		return m_baseType->deduceTemplateArgs(
			templateArgTypeArray,
			((DataPtrType*)referenceType)->getTargetType()
		);

	case TypeKind_ClassPtr:
		return m_baseType->deduceTemplateArgs(
			templateArgTypeArray,
			((ClassPtrType*)referenceType)->getTargetType()
		);

	case TypeKind_FunctionPtr:
		return m_baseType->deduceTemplateArgs(
			templateArgTypeArray,
			((FunctionPtrType*)referenceType)->getTargetType()
		);

	case TypeKind_PropertyPtr:
		return m_baseType->deduceTemplateArgs(
			templateArgTypeArray,
			((PropertyPtrType*)referenceType)->getTargetType()
		);
	}

	setTemplateArgDeductionError(referenceType);
	return false;
}

void
TemplatePtrType::prepareTypeString() {
	ASSERT(m_baseType);
	TypeStringTuple* tuple = getTypeStringTuple();

	sl::String string = m_baseType->getName();
	if (m_typeModifiers) {
		string += ' ';
		string += getTypeModifierString(m_typeModifiers);
	}

	string += '*';
	tuple->m_typeStringPrefix = string;
}

//..............................................................................

bool
TemplateIntModType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (!referenceType->getTypeKindFlags() & TypeKindFlag_Integer) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	ASSERT(m_typeModifiers & TypeModifier_Unsigned);

	TypeKind typeKind = referenceType->getTypeKind();
	TypeKind modTypeKind = getSignedIntegerTypeKind(typeKind);
	if (modTypeKind != typeKind)
		referenceType = m_module->m_typeMgr.getPrimitiveType(modTypeKind);

	return m_baseType->deduceTemplateArgs(templateArgTypeArray, referenceType);
}

void
TemplateIntModType::prepareTypeString() {
	ASSERT(m_typeModifiers);

	sl::String string = getTypeModifierString(m_typeModifiers);
	string += ' ';
	string += m_baseType->getName();
	getTypeStringTuple()->m_typeStringPrefix = string;
}

//..............................................................................

Type*
TemplateDeclType::instantiate(const sl::ArrayRef<Type*>& argArray) {
	bool result = m_declarator.getBaseType()->ensureNoImports();
	if (!result)
		return NULL;

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
