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

	Namespace* nspace = item->getNamespace();
	if (!nspace) {
		err::setFormatStringError("'%s' is a %s, not a namespace", name->getFullName ().sz(), getModuleItemKindString(item->getItemKind()));
		return false;
	}

	if (namespaceKind && namespaceKind != nspace->getNamespaceKind()) {
		err::setFormatStringError("'%s' is not %s", name->getFullName ().sz(), getNamespaceKindString(namespaceKind));
		return false;
	}

	addNamespaceImpl(nspace);
	return true;
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
FriendSet::addNamespaceImpl(Namespace* nspace) {
	sl::HashTableIterator<Namespace*, bool> it = m_friendSet.visit(nspace);
	if (it->m_value) {
		err::setFormatStringError("'%s' is already added", nspace->getDeclItem()->getItemName().sz());
		return false;
	}

	it->m_value = true;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
