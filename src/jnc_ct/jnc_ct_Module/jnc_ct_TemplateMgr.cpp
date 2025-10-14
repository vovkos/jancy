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

#include "pch.h"
#include "jnc_ct_TemplateMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

TemplateMgr::TemplateMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

void
TemplateMgr::clear() {
	m_templateList.clear();
}

Template*
TemplateMgr::createTemplate(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	Type* type,
	const sl::Array<NamedImportType*>& argArray
) {
	Template* templ = new Template;
	templ->m_module = m_module;
	templ->m_name = name;
	templ->m_qualifiedName = qualifiedName;
	templ->m_type = type;
	templ->m_argArray = argArray;
	m_templateList.insertTail(templ);
	return templ;
}

//..............................................................................

} // namespace ct
} // namespace jnc
