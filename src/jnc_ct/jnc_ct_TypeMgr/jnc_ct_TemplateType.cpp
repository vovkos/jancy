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

void
TemplateArgType::prepareSignature() {
	m_signature = sl::formatString("XA%d", m_module->createUniqueLinkId());
	m_flags |= TypeFlag_SignatureFinal;
}

sl::StringRef
TemplateArgType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix:
		return m_name;

	default:
		return TemplateType::createItemString(index);
	}
}

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

sl::StringRef
TemplateTypeName::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix:
		return TypeName::createTypeString("template ");

	default:
		return TemplateType::createItemString(index);
	}
}

bool
TemplateTypeName::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	return err::fail("TemplateTypeName::deduceTemplateArgs is not implemented");
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

sl::StringRef
TemplatePtrType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = m_baseType->getItemString(index);
		if (m_typeModifiers) {
			string += ' ';
			string += getTypeModifierString(m_typeModifiers);
		}

		string += '*';
		return string;
		}

	default:
		return TemplateType::createItemString(index);
	}
}

//..............................................................................

bool
TemplateIntModType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (!(referenceType->getTypeKindFlags() & TypeKindFlag_Integer)) {
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

sl::StringRef
TemplateIntModType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = getTypeModifierString(m_typeModifiers);
		string += ' ';
		string += m_baseType->getItemString(index);
		return string;
		}

	default:
		return TemplateType::createItemString(index);
	}
}

//..............................................................................

Type*
TemplateDeclType::instantiate(const sl::ArrayRef<Type*>& argArray) {
	Type* baseType = m_declarator.getBaseType();
	switch (baseType->getTypeKind()) {
	case TypeKind_TemplateArg:
		baseType = argArray[((TemplateArgType*)baseType)->getIndex()];
		if (!baseType) { // instantiating default type
			err::setFormatStringError("invalid reference to '%s'", ((TemplateArgType*)baseType)->getName().sz());
			return NULL;
		}

		break;

	case TypeKind_TemplateTypeName: {
		Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

		ASSERT(
			nspace->getNamespaceKind() == NamespaceKind_TemplateInstantiation ||
			nspace->getNamespaceKind() == NamespaceKind_Type &&
			((NamedType*)nspace)->getTemplateInstance()
		);

		baseType = ((TemplateTypeName*)baseType)->lookupType(nspace);
		if (!baseType)
			return NULL;

		break;
		}

	default:
		baseType = baseType->getActualTypeIfImport();
	}

	DeclTypeCalc typeCalc;
	typeCalc.setTemplateArgArray(argArray);
	uint_t declFlags;

	Type* type = typeCalc.calcType(
		baseType,
		m_declarator.getTypeModifiers(),
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

void
TemplateDeclType::prepareSignature() {
	m_signature = sl::formatString("XD%d", m_module->createUniqueLinkId());
	m_flags |= TypeFlag_SignatureFinal;
}

//..............................................................................

} // namespace ct
} // namespace jnc
