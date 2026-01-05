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
#include "jnc_ct_TypeNameFinder.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
TypeNameFinder::setup(
	Unit* parentUnit,
	Namespace* parentNamespace,
	const lex::LineCol& pos
) {
	ModuleItemContext::setup(parentUnit, parentNamespace);
	m_pos = pos;
	m_compileFlags = parentUnit->getModule()->getCompileFlags();
}

Type*
TypeNameFinder::findImpl(
	Namespace* nspace,
	const QualifiedName& name,
	bool isResolvingRecursion = false
) {
	FindModuleItemResult findResult = nspace->findItemTraverse(*this, name);
	if (!findResult.m_result) {
		pushSrcPosError();
		return NULL;
	}

	if (!findResult.m_item) {
		err::setFormatStringError("unresolved import '%s'", name.getFullName().sz());
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
		if (m_compileFlags & ModuleCompileFlag_KeepTypedefShadow)
			return ((Typedef*)item)->getShadowType();

		if (((Typedef*)item)->isRecursive() &&
			!isResolvingRecursion &&
			(nspace = ((Typedef*)item)->getGrandParentNamespace())
		)
			return findImpl(nspace, name, true);

		type = ((Typedef*)item)->getType();
		break;

	case ModuleItemKind_Template:
		type = nspace->findTemplateInstanceType((Template*)item);
		if (type)
			break;

		// else fall through

	default:
		err::setFormatStringError("'%s' is not a type", name.getFullName().sz());
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
