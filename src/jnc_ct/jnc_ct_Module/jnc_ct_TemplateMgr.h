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

#include "jnc_ct_TemplateType.h"
#include "jnc_ct_UsingSet.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Template;

//..............................................................................

struct TemplateInstance {
	ModuleItem* m_item;
	Template* m_template;
	sl::ArrayRef<Type*> m_argArray;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Template:
	public ModuleItem,
	public ModuleItemBodyDecl,
	public ModuleItemUsingSet {
	friend class TemplateMgr;

protected:
	TypeKind m_derivableTypeKind;
	TemplateDeclType* m_declType;
	sl::Array<TemplateArgType*> m_argArray;
	sl::ArrayRef<Type*> m_baseTypeArray;
	sl::StringHashTable<TemplateInstance> m_instanceMap;

public:
	Template();

	TemplateDeclType*
	getDeclType() {
		return m_declType;
	}

	const sl::Array<TemplateArgType*>&
	getArgArray() {
		return m_argArray;
	}

	bool
	isEqual(Template* templ) { // allow comparing items from different modules
		return this == templ || m_qualifiedName.isEqual(templ->m_qualifiedName);
	}

	ModuleItem*
	instantiate(const sl::ConstBoxList<Value>& argList);

	ModuleItem*
	instantiate(const sl::ArrayRef<Type*>& argArray);

	ModuleItem*
	instantiate(
		Unit* unit,
		const sl::List<Token>& argArrayTokenList
	);

	bool
	deduceArgs(
		sl::Array<Type*>* templateArgArray,
		const sl::ConstBoxList<Value>& argTypeList,
		const sl::ConstBoxList<Value>& argValueList
	);

protected:
	Namespace*
	openTemplateNamespace(const sl::ArrayRef<Type*>& argArray);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Template::Template() {
	m_itemKind = ModuleItemKind_Template;
	m_derivableTypeKind = TypeKind_Void;
	m_declType = NULL;
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
	clear();

	Template*
	createTemplate(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		TemplateDeclType* declType
	);

	Template*
	createTemplate(
		TypeKind typeKind,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		const sl::ArrayRef<TemplateArgType*>& argArray,
		const sl::ArrayRef<Type*>& baseTypeArray
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
