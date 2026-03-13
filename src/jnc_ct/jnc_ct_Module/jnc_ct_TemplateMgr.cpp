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

Template*
TemplateMgr::createTemplate(
	const sl::StringRef& name,
	TemplateDeclType* declType
) {
	Template* templ = new Template;
	templ->m_module = m_module;
	templ->m_name = name;
	templ->m_declType = declType;
	templ->m_argArray = declType->getDeclarator()->getTemplateArgArray();
	m_templateList.insertTail(templ);
	return templ;
}

Template*
TemplateMgr::createTemplate(
	TypeKind typeKind,
	const sl::StringRef& name,
	const sl::ArrayRef<TemplateArgType*>& argArray,
	const sl::ArrayRef<TemplateDeclType*>& baseTypeArray
) {
	Template* templ = new Template;
	templ->m_module = m_module;
	templ->m_name = name;
	templ->m_derivableTypeKind = typeKind;
	templ->m_argArray = argArray;
	templ->m_baseTypeArray = baseTypeArray;
	m_templateList.insertTail(templ);
	return templ;
}

Template*
TemplateMgr::createDualConstTemplate(
	const sl::StringRef& name,
	ConstKind constKind
) {
	TemplateArgType* argArray[] = {
		m_module->m_typeMgr.createTemplateArgType("T", 0), // non-const type
		m_module->m_typeMgr.createTemplateArgType("C", 1)  // const type
	};

	Template* templ = new Template;
	templ->m_module = m_module;
	templ->m_parentNamespace = m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc);
	templ->m_name = name;
	templ->m_argArray.copy(argArray, countof(argArray));
	templ->m_flags |= constKind;
	m_templateList.insertTail(templ);
	return templ;
}

//..............................................................................

} // namespace ct
} // namespace jnc
