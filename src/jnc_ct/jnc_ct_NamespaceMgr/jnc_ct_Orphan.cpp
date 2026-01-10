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

ModuleItem*
Orphan::resolveForCodeAssist(Namespace* nspace) {
	if (m_functionKind != FunctionKind_Normal && !m_declaratorNamePos)
		return resolve(nspace->getDeclItem());

	const QualifiedNameAtom& atom = m_declaratorNamePos.next(*m_declaratorName);
	FindModuleItemResult findResult = nspace->findDirectChildItem(*this, atom);
	if (!findResult.m_result || !findResult.m_item)
		return NULL;

	if (m_functionKind == FunctionKind_Normal && !m_declaratorNamePos)
		return resolve(findResult.m_item);

	nspace = findResult.m_item->getNamespace();
	return nspace ? resolveForCodeAssist(nspace) : NULL;
}

ModuleItem*
Orphan::resolve(ModuleItem* item) {
	Namespace* nspace = m_functionKind == FunctionKind_Normal ?
		item->getDecl()->getParentNamespace() :
		item->getNamespace();

	ASSERT(nspace);

	if (m_importTypeNameAnchor)
		m_importTypeNameAnchor->m_namespace = nspace;

	if (m_orphanKind == OrphanKind_Template) {
		m_module->m_namespaceMgr.openNamespace(nspace);
		Type* type = m_templateDeclType->instantiate(m_templateArgArray);
		m_module->m_namespaceMgr.closeNamespace();
		if (!type)
			return NULL;

		if (type->getTypeKind() == TypeKind_Function) {
			m_functionType = (FunctionType*)type;
			return resolveToFunction(item);
		} else if (type->getStdType() == StdType_ReactorBase)
			return resolveToReactor(item);
		else {
			err::setFormatStringError("invalid template orphan type '%s'", type->getTypeString().sz());
			return NULL;
		}
	}

	switch (m_orphanKind) {
	case OrphanKind_Function:
		return resolveToFunction(item);

	case OrphanKind_Reactor:
		return resolveToReactor(item);

	default:
		ASSERT(false);
		return NULL;
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
		(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Derivable)
	) {
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

		case FunctionKind_Getter:
			if (Property* prop = type->getIndexerProperty())
				return prop ->getGetter();
			break;

		case FunctionKind_Setter:
			if (Property* prop = type->getIndexerProperty())
				return prop->getSetter();
			break;
		}
	}

	return OverloadableFunction();
}

Function*
Orphan::resolveToFunction(ModuleItem* item) {
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
			err::setFormatStringError("'%s' is not a function", getItemName().sz());
			return NULL;
		}
	} else {
		origin = getItemUnnamedMethod(item);
		if (!origin) {
			err::setFormatStringError(
				"'%s' has no '%s'",
				item->getItemName().sz(),
				getFunctionKindString(m_functionKind)
			);

			return NULL;
		}
	}

	result =
		m_functionType->ensureLayout() &&
		origin.ensureLayout();

	if (!result)
		return NULL;

	Function* originFunction =
		origin->getItemKind() == ModuleItemKind_FunctionOverload ?
			origin.getFunctionOverload()->findShortOverload(m_functionType) :
		origin.getFunction()->getType()->getShortType()->isEqual(m_functionType) ?
			origin.getFunction() :
			NULL;

	if (!originFunction) {
		err::setFormatStringError("'%s': overload not found", getItemName().sz());
		return NULL;
	}

	if (!(originFunction->m_flags & ModuleItemFlag_User)) {
		err::setFormatStringError("'%s' is a compiler-generated function", getItemName().sz());
		return NULL;
	}

	ASSERT(originFunction->m_functionKind == m_functionKind);

	copySrcPos(originFunction);
	originFunction->addUsingSet(m_usingSet);

	FunctionType* originType = originFunction->getType();
	if (originType->getFlags() & ModuleItemFlag_User) {
		result = copyArgNames(originType);
		if (!result)
			return NULL;
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
		verifyStorageKind(originFunction) ?
			originFunction :
			NULL;
}

Function*
Orphan::resolveToReactor(ModuleItem* item) {
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
		err::setFormatStringError("'%s' is not a reactor", getItemName().sz());
		return NULL;
	}

	ReactorClassType* originType = (ReactorClassType*)itemType ;
	Function* originReactor = originType->getReactor();

	copySrcPos(originType);
	copySrcPos(originReactor);
	originReactor->addUsingSet(m_usingSet);

	return
		originType->setBody(m_pragmaConfig, m_bodyPos, m_body) &&
		verifyStorageKind(originReactor) ?
			originReactor :
			NULL;
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
	}

	return true;
}

bool
Orphan::verifyStorageKind(ModuleItemDecl* targetDecl) {
	if (!m_storageKind || m_storageKind == targetDecl->getStorageKind())
		return true;

	err::setFormatStringError("storage specifier mismatch for orphan '%s'", getItemName().sz());
	return false;
}

void
Orphan::copySrcPos(ModuleItemDecl* targetDecl) {
	targetDecl->m_parentUnit = m_parentUnit;
	targetDecl->m_pos = m_pos;
}

sl::StringRef
Orphan::createItemString(size_t index) {
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
		break;

	case ModuleItemStringKind_Synopsis: {
		sl::StringRef name = getItemString(ModuleItemStringKind_QualifiedName);
		switch (m_orphanKind) {
		case OrphanKind_Function:
			return createItemStringImpl(index, this, m_functionType, 0);

		case OrphanKind_Reactor:
			return "reactor " + name;

		case OrphanKind_Template:
			return createItemStringImpl(index, this, m_templateDeclType, 0);

		default:
			ASSERT(false);
			return sl::StringRef();
		}
		}

	default:
		return sl::StringRef();
	}

	sl::StringRef parentName = m_parentNamespace->getDeclItem()->getItemName();
	sl::StringRef declaratorName = m_declaratorName->getFullName();

	if (m_functionKind == FunctionKind_Normal)
		return parentName.isEmpty() ? declaratorName : parentName + '.' + declaratorName;

	sl::String string = parentName.isEmpty() ? declaratorName : parentName + '.' + declaratorName;
	string += '.';
	string += getFunctionKindString(m_functionKind);
	return string;
}

//..............................................................................

} // namespace ct
} // namespace jnc
