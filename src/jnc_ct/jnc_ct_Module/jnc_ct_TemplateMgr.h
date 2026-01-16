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

#include "jnc_Template.h"
#include "jnc_ct_TemplateType.h"
#include "jnc_ct_UsingSet.h"
#include "jnc_ct_Orphan.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Template;

//..............................................................................

struct TemplateInstance {
	ModuleItem* m_item;
	Template* m_template;
	sl::Array<Type*> m_argArray;

	size_t
	appendArgLinkId(sl::String* string) const;

	size_t
	appendArgString(sl::String* string) const;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Template:
	public ModuleItemWithBodyDecl,
	public ModuleItemUsingSet,
	public OrphanArray {
	friend class TemplateMgr;

protected:
	TypeKind m_derivableTypeKind;
	TemplateDeclType* m_declType;
	sl::Array<TemplateArgType*> m_argArray;
	sl::Array<TemplateDeclType*> m_baseTypeArray;
	sl::StringHashTable<TemplateInstance> m_instanceMap;

public:
	Template();

	TypeKind
	getDerivableTypeKind() const {
		return m_derivableTypeKind;
	}

	TemplateDeclType*
	getDeclType() const {
		return m_declType;
	}

	const sl::Array<TemplateArgType*>&
	getArgArray() const {
		return m_argArray;
	}

	const sl::Array<TemplateDeclType*>&
	getBaseTypeArray() const {
		return m_baseTypeArray;
	}

	ModuleItem*
	instantiate(const sl::ConstBoxList<Value>& argList);

	ModuleItem*
	instantiate(const sl::ArrayRef<Type*>& argArray);

	ModuleItem*
	instantiate(
		const ModuleItemContext& context,
		const sl::List<Token>& argArrayTokenList
	);

	bool
	deduceArgs(
		sl::Array<Type*>* templateArgArray,
		const sl::ConstBoxList<Value>& argTypeList,
		const sl::ConstBoxList<Value>& argValueList
	) const;

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	TemplateNamespace*
	openTemplateInstNamespace(const sl::ArrayRef<Type*>& argArray) const;

	void
	copyDecl(ModuleItemDecl* itemDecl) const {
		itemDecl->copyDecl(this);
	}

	void
	copyDecl(ModuleItemBodyDecl* itemDecl) const {
		copyDecl(itemDecl, this);
	}

	static
	void
	copyDecl(
		ModuleItemBodyDecl* dstDecl,
		const ModuleItemBodyDecl* srcDecl
	);

	bool
	setDefaultArgs(sl::Array<Type*>* argArray);

	ModuleItem*
	instantiateImpl(const sl::ArrayRef<Type*>& argArray);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Template::Template() {
	m_itemKind = ModuleItemKind_Template;
	m_derivableTypeKind = TypeKind_Void;
	m_declType = NULL;
}

inline
void
Template::copyDecl(
	ModuleItemBodyDecl* dstDecl,
	const ModuleItemBodyDecl* srcDecl
) {
	ASSERT(srcDecl->hasBody());
	dstDecl->copyDecl(srcDecl);
	dstDecl->copyBody(srcDecl);
}

//..............................................................................

class TemplateMgr {
protected:
	Module* m_module;
	sl::List<Template> m_templateList;

public:
	TemplateMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear() {
		m_templateList.clear();
	}

	Template*
	createTemplate(
		const sl::StringRef& name,
		TemplateDeclType* declType
	);

	Template*
	createTemplate(
		TypeKind typeKind,
		const sl::StringRef& name,
		const sl::ArrayRef<TemplateArgType*>& argArray,
		const sl::ArrayRef<TemplateDeclType*>& baseTypeArray
	);
};

//..............................................................................

inline
bool
FriendSet::isFriend(Namespace* nspace) const {
	if (m_namespaceSet.find(nspace))
		return true;

	if (nspace->getNamespaceKind() != NamespaceKind_Type || m_templateSet.isEmpty())
		return false;

	TemplateInstance* instance = ((NamedType*)nspace)->getTemplateInstance();
	return instance && m_templateSet.find(instance->m_template);
}

//..............................................................................

} // namespace ct
} // namespace jnc
