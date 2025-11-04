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
#include "jnc_ct_Orphan.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ReactorClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

Orphan::Orphan() {
	m_itemKind = ModuleItemKind_Orphan;
	m_orphanKind = OrphanKind_Undefined;
	m_functionType = NULL;
	m_origin = NULL;
}

ModuleItem*
Orphan::resolveForCodeAssist(Namespace* nspace) {
	if (m_functionKind != FunctionKind_Normal && m_declaratorName.isEmpty()) {
		adopt(nspace->getParentItem());
		return m_origin;
	}

	QualifiedNameAtom name = m_declaratorName.removeFirstName();
	FindModuleItemResult findResult = nspace->findDirectChildItem(name);
	if (!findResult.m_result || !findResult.m_item)
		return NULL;

	if (m_functionKind == FunctionKind_Normal && m_declaratorName.isEmpty()) {
		adopt(findResult.m_item);
		return m_origin;
	}

	nspace = findResult.m_item->getNamespace();
	return nspace ? resolveForCodeAssist(nspace) : NULL;
}

bool
Orphan::adopt(ModuleItem* item) {
	switch (m_orphanKind) {
	case OrphanKind_Function:
		return adoptOrphanFunction(item);

	case OrphanKind_Reactor:
		return adoptOrphanReactor(item);

	default:
		ASSERT(false);
		return true;
	}
}

OverloadableFunction
Orphan::getItemUnnamedMethod(ModuleItem* item) {
	if (item->getItemKind() == ModuleItemKind_Property) {
		Property* prop = (Property*)item;
		switch (m_functionKind) {
		case FunctionKind_Constructor:
			return prop->getConstructor();

		case FunctionKind_StaticConstructor:
			return prop->getStaticConstructor();

		case FunctionKind_Destructor:
			return prop->getDestructor();

		case FunctionKind_Getter:
			return prop->getGetter();

		case FunctionKind_Setter:
			return prop->getSetter();
		}
	} else if (
		item->getItemKind() == ModuleItemKind_Type &&
		(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Derivable)) {
		DerivableType* type = (DerivableType*)item;
		switch (m_functionKind) {
		case FunctionKind_Constructor:
			return type->getConstructor();

		case FunctionKind_StaticConstructor:
			return type->getStaticConstructor();

		case FunctionKind_Destructor:
			return type->getTypeKind() == TypeKind_Class ? ((ClassType*)type)->getDestructor() : NULL;

		case FunctionKind_UnaryOperator:
			return type->getUnaryOperator(m_unOpKind);

		case FunctionKind_BinaryOperator:
			return type->getBinaryOperator(m_binOpKind);

		case FunctionKind_CallOperator:
			return type->getCallOperator();
		}
	}

	return OverloadableFunction();
}

bool
Orphan::adoptOrphanFunction(ModuleItem* item) {
	bool result;
	OverloadableFunction origin;

	ModuleItemKind itemKind = item->getItemKind();

	if (m_functionKind == FunctionKind_Normal) {
		switch (itemKind) {
		case ModuleItemKind_Function:
			origin = (Function*)item;
			break;

		case ModuleItemKind_FunctionOverload:
			origin = (FunctionOverload*)item;
			break;

		default:
			err::setFormatStringError("'%s' is not a function", getQualifiedName().sz());
			return false;
		}
	} else {
		origin = getItemUnnamedMethod(item);
		if (!origin) {
			ModuleItemDecl* decl = item->getDecl();
			ASSERT(decl);

			err::setFormatStringError(
				"'%s' has no '%s'",
				decl->getQualifiedName().sz(),
				getFunctionKindString(m_functionKind)
			);

			return false;
		}
	}

	result =
		m_functionType->ensureLayout() &&
		origin.ensureLayout();

	if (!result)
		return false;

	Function* originFunction =
		origin->getItemKind() == ModuleItemKind_FunctionOverload ?
			origin.getFunctionOverload()->findShortOverload(m_functionType) :
		origin.getFunction()->getType()->getShortType()->cmp(m_functionType) == 0 ?
			origin.getFunction() :
			NULL;

	if (!originFunction) {
		err::setFormatStringError("'%s': overload not found", getQualifiedName().sz());
		return false;
	}

	m_origin = originFunction;

	if (!(originFunction->m_flags & ModuleItemFlag_User)) {
		err::setFormatStringError("'%s' is a compiler-generated function", getQualifiedName().sz());
		return false;
	}

	ASSERT(originFunction->m_functionKind == m_functionKind);

	copySrcPos(originFunction);
	originFunction->addUsingSet(&m_usingSet);

	FunctionType* originType = originFunction->getType();
	if (originType->getFlags() & ModuleItemFlag_User) {
		result = copyArgNames(originType);
		if (!result)
			return false;
	} else {
		sl::Array<FunctionArg*> argArray = m_functionType->getArgArray();
		if (originType->isMemberMethodType())
			argArray.insert(0, originType->getThisArg());

		originFunction->m_type = m_module->m_typeMgr.createUserFunctionType(
			originType->getCallConv(),
			originType->getReturnType(),
			argArray,
			originType->getFlags() & FunctionTypeFlag__All
		);
	}

	return
		originFunction->setBody(m_pragmaConfig, m_bodyPos, m_body) &&
		verifyStorageKind(originFunction);
}

bool
Orphan::adoptOrphanReactor(ModuleItem* item) {
	Type* itemType = NULL;

	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Variable:
		itemType = ((Variable*)item)->getType();
		break;

	case ModuleItemKind_Field:
		itemType = ((Field*)item)->getType();
		break;
	}

	if (!itemType || !isClassType(itemType, ClassTypeKind_Reactor)) {
		err::setFormatStringError("'%s' is not a reactor", getQualifiedName().sz());
		return false;
	}

	ReactorClassType* originType = (ReactorClassType*)itemType ;
	Function* originReactor = originType->getReactor();
	m_origin = originReactor;

	copySrcPos(originType);
	copySrcPos(originReactor);
	originReactor->addUsingSet(&m_usingSet);

	return
		originType->setBody(m_pragmaConfig, m_bodyPos, m_body) &&
		verifyStorageKind(originReactor);
}

bool
Orphan::copyArgNames(FunctionType* targetFunctionType) {
	ASSERT(targetFunctionType->getFlags() & ModuleItemFlag_User);

	// copy arg names and make sure orphan funciton does not override default values

	sl::Array<FunctionArg*> dstArgArray = targetFunctionType->getArgArray();
	sl::Array<FunctionArg*> srcArgArray = m_functionType->getArgArray();

	size_t argCount = dstArgArray.getCount();

	size_t iDst = 0;
	size_t iSrc = 0;

	if (targetFunctionType->isMemberMethodType())
		iDst++;

	for (; iDst < argCount; iDst++, iSrc++) {
		FunctionArg* dstArg = dstArgArray[iDst];
		FunctionArg* srcArg = srcArgArray[iSrc];

		if (!srcArg->m_initializer.isEmpty()) {
			err::setFormatStringError("redefinition of default value for '%s'", srcArg->m_name.sz());
			return false;
		}

		dstArg->m_name = srcArg->m_name;
		dstArg->m_qualifiedName = srcArg->m_qualifiedName;
	}

	return true;
}

bool
Orphan::verifyStorageKind(ModuleItemDecl* targetDecl) {
	if (!m_storageKind || m_storageKind == targetDecl->getStorageKind())
		return true;

	err::setFormatStringError("storage specifier mismatch for orphan '%s'", getQualifiedName().sz());
	return false;
}

void
Orphan::copySrcPos(ModuleItemDecl* targetDecl) {
	targetDecl->m_parentUnit = m_parentUnit;
	targetDecl->m_pos = m_pos;
}

//..............................................................................

} // namespace ct
} // namespace jnc
