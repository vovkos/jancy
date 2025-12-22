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
#include "jnc_ct_Orphan.h"

namespace jnc {
namespace ct {

class DerivableType;
class EnumType;
class Const;
class Value;
class MemberCoord;
class MemberBlock;

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
	public ModuleItemUsingSet,
	public OrphanArray {
	friend class NamespaceMgr;
	friend class TypeMgr;
	friend class Parser;

protected:
	NamespaceKind m_namespaceKind;
	NamespaceStatus m_namespaceStatus;
	err::Error m_parseError;
	sl::Array<ModuleItem*> m_itemArray;
	sl::StringHashTable<ModuleItem*> m_itemMap;
	sl::StringHashTable<DualPtrTypeTuple*> m_dualPtrTypeTupleMap;

public:
	Namespace() {
		m_namespaceKind = NamespaceKind_Undefined;
		m_namespaceStatus = NamespaceStatus_ParseRequired; // most namespaces are lazily parsed
	}

	NamespaceKind
	getNamespaceKind() const {
		return m_namespaceKind;
	}

	bool
	isNamespaceReady() const {
		return m_namespaceStatus == NamespaceStatus_Ready;
	}

	virtual
	MemberBlock*
	getMemberBlock() {
		return NULL;
	}

	bool
	ensureNamespaceReady();

	bool
	ensureNamespaceReadyDeep();

	bool
	parseLazyImports();

	FindModuleItemResult
	findDirectChildItem(const sl::StringRef& name);

	FindModuleItemResult
	findDirectChildItem(const QualifiedNameAtom& atom) {
		return findDirectChildItem(ModuleItemContext(), atom);
	}

	FindModuleItemResult
	findDirectChildItem(
		const ModuleItemContext& context,
		const QualifiedNameAtom& atom
	);

	FindModuleItemResult
	findItem(const QualifiedName& name) {
		return findItem(ModuleItemContext(), name);
	}

	FindModuleItemResult
	findItem(
		const ModuleItemContext& context,
		const QualifiedName& name
	);

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
		const QualifiedNameAtom& atom,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	) {
		return findDirectChildItemTraverse(ModuleItemContext(), atom, coord, flags);
	}

	FindModuleItemResult
	findDirectChildItemTraverse(
		const ModuleItemContext& context,
		const QualifiedNameAtom& atom,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	);

	FindModuleItemResult
	findItemTraverse(
		const QualifiedName& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	) {
		return findItemTraverse(ModuleItemContext(), name, coord, flags);
	}

	FindModuleItemResult
	findItemTraverse(
		const ModuleItemContext& context,
		const QualifiedName& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	);

	NamedType*
	findTemplateInstanceType(Template* templ);

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
		const ModuleItemContext& context,
		const QualifiedNameAtom& atom,
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

template <typename T = ModuleItem>
class ModuleItemWithNamespace:
	public T,
	public Namespace
{
public:
	virtual
	ModuleItem*
	getDeclItem() {
		return this;
	}

	virtual
	ModuleItemDecl*
	getDecl() {
		return this;
	}

	virtual
	Namespace*
	getNamespace() {
		return this;
	}

protected:
	virtual
	sl::StringRef
	createLinkId() {
		return createLinkIdImpl(this->m_module);
	}

	virtual
	sl::StringRef
	createItemString(size_t index) {
		return createItemStringImpl(index, this); // minimal default fallback
	}
};

//..............................................................................

class TemplateNamespace: public ModuleItemWithNamespace<> {
	friend class NamespaceMgr;
	DerivableType* m_instanceType;

public:
	TemplateNamespace(NamespaceKind namespaceKind);

	DerivableType*
	getInstanceType() const {
		return m_instanceType;
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TemplateNamespace::TemplateNamespace(NamespaceKind namespaceKind) {
	m_itemKind = ModuleItemKind_TemplateNamespace;
	m_namespaceKind = namespaceKind;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_instanceType = NULL;
}

//..............................................................................

inline
void
ModuleItemUsingSet::addUsingSet(Namespace* anchorNamespace) {
	for (Namespace* nspace = anchorNamespace; nspace; nspace = nspace->getParentNamespace())
		m_usingSet.append(nspace->getUsingSet());
}

//..............................................................................

inline
Namespace*
ModuleItemContext::getGrandParentNamespace() const {
	return m_parentNamespace ? m_parentNamespace->getParentNamespace() : NULL;
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
