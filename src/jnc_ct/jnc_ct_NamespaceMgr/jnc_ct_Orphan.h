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

#include "jnc_ct_UsingSet.h"
#include "jnc_ct_FunctionName.h"

namespace jnc {
namespace ct {

class FunctionType;
class TemplateDeclType;
struct NamedImportAnchor;

//..............................................................................

enum OrphanKind {
	OrphanKind_Undefined,
	OrphanKind_Function,
	OrphanKind_Reactor,
	OrphanKind_Template,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Orphan:
	public ModuleItemWithBodyDecl,
	public ModuleItemUsingSet,
	public FunctionName {
	friend class NamespaceMgr;
	friend class Namespace;
	friend class Parser;

protected:
	OrphanKind m_orphanKind;
	QualifiedName m_declaratorName;
	QualifiedNamePos m_declaratorNamePos;
	NamedImportAnchor* m_namedImportAnchor;

	union {
		Type* m_type;
		FunctionType* m_functionType;
		TemplateDeclType* m_templateDeclType;
	};

	sl::Array<Type*> m_templateArgArray;

public:
	Orphan();

	OrphanKind
	getOrphanKind() const {
		return m_orphanKind;
	}

	const QualifiedName&
	getDeclaratorName() const {
		return m_declaratorName;
	}

	NamedImportAnchor*
	getNamedImportAnchor() {
		return m_namedImportAnchor;
	}

	Type*
	getType() const {
		return m_type;
	}

	const sl::Array<Type*>&
	getTemplateArgArray() const {
		return m_templateArgArray;
	}

	size_t
	appendTemplateArgArray(const sl::ArrayRef<Type*>& argArray) {
		return m_templateArgArray.append(argArray);
	}

	ModuleItem*
	adopt(ModuleItem* item);

	ModuleItem*
	resolveForCodeAssist() {
		return resolveForCodeAssist(m_parentNamespace);
	}

	virtual
	Type*
	getItemType() {
		return m_type;
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	ModuleItem*
	resolveForCodeAssist(Namespace* nspace);

	Function*
	adoptOrphanFunction(ModuleItem* item);

	Function*
	adoptOrphanReactor(ModuleItem* item);

	bool
	verifyStorageKind(ModuleItemDecl* targetDecl);

	void
	copySrcPos(ModuleItemDecl* targetDecl);

	bool
	copyArgNames(FunctionType* targetFunctionType);

	OverloadableFunction
	getItemUnnamedMethod(ModuleItem* item);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Orphan::Orphan() {
	m_itemKind = ModuleItemKind_Orphan;
	m_orphanKind = OrphanKind_Undefined;
	m_functionKind = FunctionKind_Normal;
	m_namedImportAnchor = NULL;
	m_functionType = NULL;
	m_templateDeclType = NULL;
}

//..............................................................................

class OrphanArray {
protected:
	sl::Array<Orphan*> m_orphanArray;

public:
	void
	addOrphan(Orphan* orphan) {
		m_orphanArray.append(orphan);
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
