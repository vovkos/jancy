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

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_QualifiedName.h"
#include "jnc_ct_UsingSet.h"
#include "jnc_Namespace.h"

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

class Namespace: public ModuleItemBodyDecl {
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
	UsingSet m_usingSet;

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

	UsingSet*
	getUsingSet() {
		return &m_usingSet;
	}

	virtual
	sl::String
	createQualifiedName(const sl::StringRef& name);

	sl::String
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
	findItem(const QualifiedName& name);

	FindModuleItemResult
	findItem(const sl::StringRef& name);

	virtual
	FindModuleItemResult
	findDirectChildItemTraverse(
		const sl::StringRef& name,
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
	void
	clear();

	bool
	addItem(
		const sl::StringRef& name,
		ModuleItem* item
	);

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

//..............................................................................

JNC_INLINE
err::Error
setRedefinitionError(const sl::StringRef& name) {
	return err::setFormatStringError("redefinition of '%s'", name.sz());
}

//..............................................................................

} // namespace ct
} // namespace jnc
