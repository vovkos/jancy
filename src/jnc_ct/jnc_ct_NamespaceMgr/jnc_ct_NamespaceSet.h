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

#pragma once

#include "jnc_Namespace.h"
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_QualifiedName.h"

namespace jnc {
namespace ct {

//..............................................................................

class NamespaceSet {
protected:
	struct ImportNamespace: sl::ListLink {
		ModuleItemContext m_context;
		NamespaceKind m_namespaceKind;
		QualifiedName m_name;
	};

protected:
	sl::List<ImportNamespace> m_importNamespaceList;

public:
	void
	clear() {
		m_importNamespaceList.clear();
	}

	bool
	ensureResolved() const {
		return m_importNamespaceList.isEmpty() ? true : ((NamespaceSet*)this)->resolve();
	}

	bool
	addNamespace(
		const ModuleItemContext& context,
		NamespaceKind namespaceKind,
		QualifiedName* name // destructive
	);

protected:
	void
	append(const NamespaceSet& src);

	virtual
	bool
	addNamespaceImpl(
		ModuleItem* item,
		NamespaceKind namespaceKind
	) = 0;

	bool
	resolve();
};

//..............................................................................

class FriendSet: public NamespaceSet {
protected:
	sl::SimpleHashTable<Namespace*, bool> m_namespaceSet;
	sl::SimpleHashTable<Template*, bool> m_templateSet;

public:
	bool
	isFriend(Namespace* nspace) const;

protected:
	virtual
	bool
	addNamespaceImpl(
		ModuleItem* item,
		NamespaceKind namespaceKind
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
