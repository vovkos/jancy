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
	addNamespaceImpl(Namespace* nspace) = 0;

	bool
	resolve();
};

//..............................................................................

class FriendSet: public NamespaceSet {
protected:
	sl::SimpleHashTable<Namespace*, bool> m_friendSet;

public:
	bool
	isFriend(Namespace* nspace) const {
		return m_friendSet.find(nspace);
	}

protected:
	virtual
	bool
	addNamespaceImpl(Namespace* nspace);
};

//..............................................................................

} // namespace ct
} // namespace jnc
