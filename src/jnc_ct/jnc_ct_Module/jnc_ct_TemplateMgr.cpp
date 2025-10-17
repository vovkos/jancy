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
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ModuleItem*
Template::instantiate(const sl::ConstBoxList<Value>& argList) {
	size_t argCount = argList.getCount();
	if (argCount != m_argArray.getCount()) {
		err::setError("incorrect number of template arguments");
		return NULL;
	}

	sl::String signature;
	sl::ConstBoxIterator<Value> it = argList.getHead();
	for (; it; it++)
		signature += it->getType()->getSignature();

	sl::StringHashTableIterator<ModuleItem*> mapIt = m_instantiationMap.visit(signature);
	if (mapIt->m_value)
		return mapIt->m_value;

	if (!m_declType) {
		err::setError("only templated functions are currently supported");
		return NULL;
	}

	char buffer[256];
	sl::Array<Type*> argArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	argArray.setCount(argCount);
	sl::Array<Type*>::Rwi rwi = argArray.rwi();
	it = argList.getHead();
	for (size_t i = 0; it; i++, it++)
		rwi[i] = it->getType();

	Type* type = m_declType->instantiate(argArray);
	if (!type)
		return NULL;

	if (type->getTypeKind() != TypeKind_Function) {
		err::setError("only templated functions are currently supported");
		return NULL;
	}

	Function* function = m_module->m_functionMgr.createFunction((FunctionType*)type);
	function->copyDecl(this);

	if (!m_body.isEmpty())
		function->setBody(m_pragmaConfig, m_bodyPos, m_body);
	else {
		ASSERT(!m_bodyTokenList.isEmpty());
		sl::List<Token> body;
		cloneTokenList(&body, m_bodyTokenList);
		function->setBody(m_pragmaConfig, &body);
	}

	mapIt->m_value = function;
	return function;
}

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
	TemplateInstanceType* declType
) {
	Template* templ = new Template;
	templ->m_module = m_module;
	templ->m_name = name;
	templ->m_qualifiedName = qualifiedName;
	templ->m_declType = declType;
	templ->m_argArray = declType->getDeclarator()->getTemplateArgArray();
	m_templateList.insertTail(templ);
	return templ;
}

Template*
TemplateMgr::createTemplate(
	TypeKind typeKind,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	const sl::ArrayRef<TemplateArgType*>& argArray,
	const sl::ArrayRef<Type*>& baseTypeArray
) {
	Template* templ = new Template;
	templ->m_module = m_module;
	templ->m_name = name;
	templ->m_qualifiedName = qualifiedName;
	templ->m_derivableTypeKind = typeKind;
	templ->m_argArray = argArray;
	templ->m_baseTypeArray = baseTypeArray;
	m_templateList.insertTail(templ);
	return templ;
}

//..............................................................................

} // namespace ct
} // namespace jnc
