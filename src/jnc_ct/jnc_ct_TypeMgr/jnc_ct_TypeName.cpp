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
#include "jnc_ct_TypeName.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
TypeName::lookupTypeImpl(
	const ModuleItemContext& context,
	Namespace* nspace,
	uint_t compileFlags,
	bool isResolvingRecursion
) const {
	ASSERT(nspace->getNamespaceKind() != NamespaceKind_TemplateDeclaration);

	FindModuleItemResult findResult = nspace->findItemTraverse(context, m_name);
	if (!findResult.m_result) {
		pushSrcPosError();
		return NULL;
	}

	if (!findResult.m_item) {
		err::setFormatStringError("unresolved import '%s'", m_name.getFullName().sz());
		pushSrcPosError();
		return NULL;
	}

	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();
	Type* type;
	switch (itemKind) {
	case ModuleItemKind_Type:
		type = (Type*)item;
		break;

	case ModuleItemKind_Typedef:
		if (compileFlags & ModuleCompileFlag_KeepTypedefShadow)
			return ((Typedef*)item)->getShadowType();

		if (((Typedef*)item)->isRecursive() &&
			!isResolvingRecursion &&
			(nspace = ((Typedef*)item)->getGrandParentNamespace())
		)
			return lookupTypeImpl(context, nspace, compileFlags, true);

		type = ((Typedef*)item)->getType();
		break;

	case ModuleItemKind_Template:
		type = nspace->findTemplateInstanceType((Template*)item);
		if (type)
			break;

		// else fall through

	default:
		err::setFormatStringError("'%s' is not a type", m_name.getFullName().sz());
		pushSrcPosError();
		return NULL;
	}

	if (!(type->getTypeKindFlags() & TypeKindFlag_Import))
		return type;

	ImportType* importType = (ImportType*)type;
	bool result = importType->ensureResolved();
	if (!result)
		return NULL;

	type = importType->getActualType();
	ASSERT(!(type->getTypeKindFlags() & TypeKindFlag_Import));
	return type;
};

//..............................................................................

} // namespace ct
} // namespace jnc
