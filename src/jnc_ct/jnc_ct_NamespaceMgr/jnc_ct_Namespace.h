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
#include "jnc_ct_UsingSet.h"

namespace jnc {
namespace ct {

class DerivableType;
class EnumType;
class Const;
class MemberCoord;
class Value;
class Orphan;

struct DualPtrTypeTuple;

//..............................................................................

enum {
	TraverseFlag_NoThis                = 0x01,
	TraverseFlag_NoBaseType            = 0x04,
	TraverseFlag_NoParentNamespace     = 0x08,
	TraverseFlag_NoUsingNamespaces     = 0x10,
	TraverseFlag_NoExtensionNamespaces = 0x20,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum NamespaceStatus {
	NamespaceStatus_ParseRequired = 0,
	NamespaceStatus_Parsing       = 1,
	NamespaceStatus_ParseError    = -1,
	NamespaceStatus_Ready         = 2,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Namespace:
	public ModuleItemBodyDecl,
	public ModuleItemUsingSet {
	friend class NamespaceMgr;
	friend class TypeMgr;
	friend class Parser;

protected:
	NamespaceKind m_namespaceKind;
	NamespaceStatus m_namespaceStatus;
	err::Error m_parseError;
	sl::Array<ModuleItem*> m_itemArray;
	sl::Array<Orphan*> m_orphanArray;
	sl::StringHashTable<ModuleItem*> m_itemMap;
	sl::StringHashTable<bool> m_friendSet;
	sl::StringHashTable<DualPtrTypeTuple*> m_dualPtrTypeTupleMap;

public:
	Namespace() {
		m_namespaceKind = NamespaceKind_Undefined;
		m_namespaceStatus = NamespaceStatus_ParseRequired; // most namespaces are lazily parsed
	}

	NamespaceKind
	getNamespaceKind() {
		return m_namespaceKind;
	}

	bool
	isNamespaceReady() {
		return m_namespaceStatus == NamespaceStatus_Ready;
	}

	bool
	ensureNamespaceReady();

	bool
	ensureNamespaceReadyDeep();

	bool
	parseLazyImports();

	ModuleItem*
	getParentItem();

	virtual
	sl::StringRef
	createQualifiedName(const sl::StringRef& name);

	sl::StringRef
	createQualifiedName(const QualifiedName& name) {
		return createQualifiedName(name.getFullName());
	}

	bool
	isFriend(Namespace* nspace) {
		return m_friendSet.find(nspace->getQualifiedName()) != NULL;
	}

	FindModuleItemResult
	findDirectChildItem(const sl::StringRef& name);

	FindModuleItemResult
	findDirectChildItem(
		Unit* unit,
		const QualifiedNameAtom& name
	);

	FindModuleItemResult
	findItem(const QualifiedName& name);

	FindModuleItemResult
	findItem(const sl::StringRef& name) {
		return findItemImpl<sl::True>(name);
	}

	FindModuleItemResult
	findItemNoParse(const sl::StringRef& name) {
		return findItemImpl<sl::False>(name);
	}

	virtual
	FindModuleItemResult
	findDirectChildItemTraverse(
		const sl::StringRef& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	);

	FindModuleItemResult
	findDirectChildItemTraverse(
		Unit* unit,
		const QualifiedNameAtom& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	);

	FindModuleItemResult
	findItemTraverse(
		const QualifiedName& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	);

	template <typename T>
	bool
	addItem(T* item) {
		return addItem(item->getName(), item);
	}

	bool
	addItem(
		const sl::StringRef& name,
		ModuleItem* item
	);

	void
	replaceItem(
		const sl::StringRef& name,
		ModuleItem* item
	) {
		m_itemMap.visit(name)->m_value = item;
	}

	size_t
	addFunction(Function* function); // returns overload idx or -1 on error

	void
	addOrphan(Orphan* orphan) {
		m_orphanArray.append(orphan);
	}

	Const*
	createConst(
		const sl::StringRef& name,
		const Value& value
	);

	const sl::Array<ModuleItem*>&
	getItemArray() {
		return m_itemArray;
	}

	bool
	exposeEnumConsts(EnumType* member);

	bool
	resolveOrphans();

protected:
	template <typename CanParse>
	FindModuleItemResult
	findItemImpl(const sl::StringRef& name);

	FindModuleItemResult
	finalizeFindTemplate(
		Unit* unit,
		const QualifiedNameAtom& name,
		FindModuleItemResult findResult
	);

	void
	clear();

	virtual
	bool
	parseBody() {
		ASSERT(false);
		return true;
	}

	bool
	generateMemberDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml,
		bool useSectionDef
	);
};


// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename CanParse>
FindModuleItemResult
Namespace::findItemImpl(const sl::StringRef& name) {
	const char* p = name.cp();
	const char* end = name.getEnd();

	Namespace* nspace = this;
	FindModuleItemResult findResult = g_nullFindModuleItemResult;
	for (;;) {
		if (!CanParse()() && !nspace->isNamespaceReady())
			return g_nullFindModuleItemResult;

		size_t length = end - p;
		const char* dot = (const char*)memchr(p, '.', length);
		if (!dot)
			return nspace->findDirectChildItem(sl::StringRef(p, length));

		findResult = nspace->findDirectChildItem(sl::StringRef(p, dot - p));
		if (!findResult.m_item)
			return findResult;

		nspace = findResult.m_item->getNamespace();
		if (!nspace)
			return g_nullFindModuleItemResult;

		p = dot + 1;
	}
}

//..............................................................................

inline
void
ModuleItemDecl::prepareQualifiedName() {
	ASSERT(m_qualifiedName.isEmpty());
	m_qualifiedName = m_parentNamespace ? m_parentNamespace->createQualifiedName(m_name) : m_name;
}

//..............................................................................

JNC_INLINE
err::Error
setRedefinitionError(const sl::StringRef& name) {
	return err::setFormatStringError("redefinition of '%s'", name.sz());
}

//..............................................................................

} // namespace ct
} // namespace jnc
