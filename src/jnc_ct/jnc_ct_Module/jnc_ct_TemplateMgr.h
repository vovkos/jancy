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

#include "jnc_ct_Template.h"

namespace jnc {
namespace ct {

//..............................................................................

class TemplateMgr {
protected:
	Module* m_module;
	sl::List<Template> m_templateList;
	Template* m_autoConstTemplate;

public:
	TemplateMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear() {
		m_templateList.clear();
		m_autoConstTemplate = NULL;
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

	Template*
	getAutoConstTemplate() {
		return m_autoConstTemplate ? m_autoConstTemplate : createAutoConstTemplate();
	}

protected:
	Template*
	createAutoConstTemplate();
};

//..............................................................................

} // namespace ct
} // namespace jnc
