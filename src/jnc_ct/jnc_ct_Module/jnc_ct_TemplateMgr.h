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

//..............................................................................

class Template:
	public ModuleItem,
	public ModuleItemBodyDecl,
	public ModuleItemUsingSet {
	friend class TemplateMgr;

protected:
	TypeKind m_typeKind;
	TemplateDeclType* m_declType;
	sl::Array<TemplateArgType*> m_argArray;
	sl::StringHashTable<ModuleItem*> m_instantiationMap;

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

	ModuleItem*
	instantiate(const sl::ConstBoxList<Value>& argList);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Template::Template() {
	m_itemKind = ModuleItemKind_Template;
	m_typeKind = TypeKind_Void;
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
		TemplateDeclType* type
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
