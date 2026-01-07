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
#include "jnc_ct_NamespaceSet.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
NamespaceSet::append(const NamespaceSet& src) {
	sl::ConstIterator<ImportNamespace> it = src.m_importNamespaceList.getHead();
	for (; it; it++) {
		ImportNamespace* importNamespace = new ImportNamespace;
		importNamespace->m_context = it->m_context;
		importNamespace->m_namespaceKind = it->m_namespaceKind;
		importNamespace->m_name.copy(it->m_name);
		m_importNamespaceList.insertTail(importNamespace);
	}
}

bool
NamespaceSet::addNamespace(
	const ModuleItemContext& context,
	NamespaceKind namespaceKind,
	QualifiedName* name
) {
	ModuleItem* item;
	Module* module = context.getParentUnit()->getModule();
	if (module->getCompileState() >= ModuleCompileState_Parsed) {
		FindModuleItemResult findResult = context.getParentNamespace()->findItemTraverse(context, *name);
		if (!findResult.m_result)
			return false;

		if (!findResult.m_item) {
			err::setFormatStringError("'%s' not found", name->getFullName().sz());
			return false;
		}

		item = findResult.m_item;
	} else {
		if (!name->isSimple())
			item = NULL;
		else {
			FindModuleItemResult findResult = context.getParentNamespace()->findDirectChildItem(context, name->getFirstAtom());
			if (!findResult.m_result)
				return false;

			item = findResult.m_item;
		}

		if (!item) {
			ImportNamespace* importNamespace = new ImportNamespace;
			importNamespace->m_context = context;
			importNamespace->m_namespaceKind = namespaceKind;
			sl::takeOver(&importNamespace->m_name, name);
			m_importNamespaceList.insertTail(importNamespace);
			return true;
		}
	}

	return addNamespaceImpl(item, namespaceKind);
}

bool
NamespaceSet::resolve() {
	ASSERT(
		m_importNamespaceList.isEmpty() ||
		m_importNamespaceList.getHead()->m_context.getParentUnit()->getModule()->getCompileState() >= ModuleCompileState_Parsed
	);

	bool result = true;

	while (!m_importNamespaceList.isEmpty()) {
		ImportNamespace* importNamespace = m_importNamespaceList.removeHead();

		result = addNamespace(
			importNamespace->m_context,
			importNamespace->m_namespaceKind,
			&importNamespace->m_name
		) && result;

		delete importNamespace;
	}

	return result;
}

//..............................................................................

bool
FriendSet::addNamespaceImpl(
	ModuleItem* item,
	NamespaceKind namespaceKind
) {
	bool* p;
	if (item->getItemKind() == ModuleItemKind_Template)
		p = &m_templateSet.visit((Template*)item)->m_value;
	else {
		Namespace* nspace = item->getNamespace();
		if (nspace)
			p = &m_namespaceSet.visit(nspace)->m_value;
		else {
			err::setFormatStringError("'%s' can't be a friend", item->getItemName().sz());
			return false;
		}
	}

	if (*p) {
		err::setFormatStringError("'%s' is already a friend", item->getItemName().sz());
		return false;
	}

	*p = true;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
