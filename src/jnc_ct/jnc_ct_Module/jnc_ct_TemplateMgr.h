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

namespace jnc {
namespace ct {

class NamedImportType;

//..............................................................................

class Template:
	public ModuleItem,
	public ModuleItemBodyDecl,
	public ModuleItemUsingSet {
	friend class TemplateMgr;

protected:
	Type* m_type;
	sl::Array<NamedImportType*> m_argArray;
	sl::StringHashTable<ModuleItem*> m_instantiationMap;

public:
	Template() {
		m_itemKind = ModuleItemKind_Template;
		m_type = NULL;
	}

	Type*
	getType() {
		return m_type;
	}

	const sl::Array<NamedImportType*>&
	getArgArray() {
		return m_argArray;
	}
};

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
		Type* type,
		const sl::Array<NamedImportType*>& argArray
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
