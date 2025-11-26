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
#include "jnc_ct_UnionType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ParseContext.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

size_t
TemplateInstance::appendArgLinkId(sl::String* string) const {
	*string += '<';

	size_t argCount = m_argArray.getCount();
	for (size_t i = 0; i < argCount; i++)
		*string += m_argArray[i]->getSignature();

	*string += '>';
	return string->getLength();
}

size_t
TemplateInstance::appendArgString(sl::String* string) const {
	*string += '<';

	*string += m_argArray[0]->getTypeString();
	size_t argCount = m_argArray.getCount();
	for (size_t i = 1; i < argCount; i++) {
		*string += ", ";
		*string += m_argArray[i]->getSignature();
	}

	*string += '>';
	return string->getLength();
}

//..............................................................................

ModuleItem*
Template::instantiate(const sl::ConstBoxList<Value>& argList) {
	size_t argCount = argList.getCount();

	char buffer[256];
	sl::Array<Type*> argArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	argArray.setCount(argCount);
	sl::Array<Type*>::Rwi rwi = argArray.rwi();
	sl::ConstBoxIterator<Value> it = argList.getHead();
	for (size_t i = 0; it; i++, it++)
		rwi[i] = it->getType();

	return instantiate(argArray);
}

ModuleItem*
Template::instantiate(const sl::ArrayRef<Type*>& argArray) {
	size_t argCount = argArray.getCount();
	if (argCount != m_argArray.getCount()) {
		err::setError("incorrect number of template arguments");
		return NULL;
	}

	sl::String signature;
	for (size_t i = 0; i < argCount; i++)
		signature += argArray[i]->getSignature();

	sl::StringHashTableIterator<TemplateInstance> mapIt = m_instanceMap.visit(signature);
	TemplateInstance* instance = &mapIt->m_value;
	if (instance->m_item)
		return instance->m_item;

	ModuleItem* item;

	if (m_declType) {
		openTemplateNamespace(argArray);
		Type* type = m_declType->instantiate(argArray);
		m_module->m_namespaceMgr.closeTemplateNamespace();
		if (!type)
			return NULL;

		if (m_storageKind == StorageKind_Typedef) {
			Typedef* tdef = m_module->m_typeMgr.createTypedef(m_name, type);
			copyDecl(tdef);
			item = tdef;
		} else {
			if (type->getTypeKind() != TypeKind_Function) {
				err::setError("cannot instantiate template '%s' (not a function)");
				return NULL;
			}

			Function* function = m_module->m_functionMgr.createFunction((FunctionType*)type);
			function->m_templateInstance = &mapIt->m_value;
			copyDecl(function);
			item = function;
		}
	} else {
		DerivableType* type;
		switch (m_derivableTypeKind) {
		case TypeKind_Struct:
			type = m_module->m_typeMgr.createStructType(m_name, m_pragmaConfig->m_fieldAlignment);
			break;

		case TypeKind_Union:
			type = m_module->m_typeMgr.createUnionType(m_name, m_pragmaConfig->m_fieldAlignment);
			break;

		case TypeKind_Class:
			type = m_module->m_typeMgr.createClassType(m_name, m_pragmaConfig->m_fieldAlignment);
			break;

		default:
			ASSERT(false);
			return NULL;
		}

		for (size_t i = 0; i < argCount; i++) {
			const sl::StringRef& name = m_argArray[i]->getName();
			Typedef* tdef = m_module->m_typeMgr.createTypedef(name, argArray[i]);
			bool result = type->addItem(tdef);
			ASSERT(result); // should have been checked in parser
		}

		size_t orphanCount = m_orphanArray.getCount();
		for (size_t i = 0; i < orphanCount; i++) {
			Orphan* srcOrphan = m_orphanArray[i];
			Orphan* orphan = m_module->m_namespaceMgr.cloneOrphan(srcOrphan);
			copyDecl(orphan, srcOrphan);
			type->addOrphan(orphan);
		}

		type->m_templateInstance = instance;
		copyDecl(type);
		item = type;
	}

	instance->m_item = item;
	instance->m_template = this;
	instance->m_argArray = argArray;
	return item;
}

ModuleItem*
Template::instantiate(
	const ModuleItemContext& context,
	const sl::List<Token>& argArrayTokenList
) {
	sl::List<Token> tokenList;
	cloneTokenList(&tokenList, argArrayTokenList);

	ParseContext parseContext(ParseContextKind_TypeName, m_module, context);
	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Compile);
	bool result = parser.parseTokenList(SymbolKind_type_name_list_save, &tokenList);
	return result ? instantiate(parser.getLastTypeArray()) : NULL;
}

bool
Template::deduceArgs(
	sl::Array<Type*>* templateArgArray,
	const sl::ConstBoxList<Value>& argTypeList,
	const sl::ConstBoxList<Value>& argValueList
) {
	Type* deductionType = m_declType->getDeductionType();
	if (!deductionType) {
		openTemplateNamespace(*(sl::Array<Type*>*)&m_argArray);
		deductionType = m_declType->createDeductionType();
		m_module->m_namespaceMgr.closeTemplateNamespace();
		if (!deductionType)
			return false;
	}

	if (deductionType->getTypeKind() != TypeKind_Function) {
		err::setFormatStringError("cannot deduce arguments of template '%s' (not a function)", getItemName().sz());
		return false;
	}

	FunctionType* functionType = (FunctionType*)deductionType;
	const sl::Array<FunctionArg*>& functionArgArray = functionType->getArgArray();
	size_t functionArgCount = functionArgArray.getCount();
	if (functionArgCount != argValueList.getCount()) {
		err::setFormatStringError("'%s' does not take %d arguments", getItemName().sz(), argValueList.getCount());
		return false;
	}

	size_t templateArgCount = m_argArray.getCount();
	templateArgArray->setCountZeroConstruct(templateArgCount);

	bool result = true;
	sl::ConstBoxIterator<Value> argValueIt = argValueList.getHead();
	for (size_t i = 0; i < functionArgCount; i++, argValueIt++) {
		Value argTypeValue;

		result =
			m_module->m_operatorMgr.prepareOperandType(*argValueIt, &argTypeValue) &&
			functionArgArray[i]->getType()->deduceTemplateArgs(templateArgArray, argTypeValue.getType()) &&
			result;
	}

	for (size_t i = 0; i < templateArgCount; i++) {
		if (!(*templateArgArray)[i]) {
			err::setFormatStringError(
				"cannot deduce argument '%s' of template '%s'",
				m_argArray[i]->getName().sz(),
				getItemName().sz()
			);

			return false;
		}
	}

	return result;
}

sl::StringRef
Template::createItemString(size_t index) {
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
		break;
	case ModuleItemStringKind_Synopsis:
		return "template " + getItemName();
	default:
		return sl::StringRef();
	}

	sl::String string = createQualifiedNameImpl(m_module);
	string += '<';

	size_t count = m_argArray.getCount();
	ASSERT(count);

	string += m_argArray[0]->getName();
	for (size_t i = 1; i < count; i++) {
		string += ", ";
		string += m_argArray[i]->getName();
	}

	string += '>';
	return string;
}

Namespace*
Template::openTemplateNamespace(const sl::ArrayRef<Type*>& argArray) {
	Namespace* nspace = m_module->m_namespaceMgr.openTemplateNamespace();

	size_t argCount = argArray.getCount();
	for (size_t i = 0; i < argCount; i++) {
		bool result = nspace->addItem(m_argArray[i]->getName(), argArray[i]);
		ASSERT(result);
	}

	return nspace;
}

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
	const sl::ArrayRef<Type*>& baseTypeArray
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

//..............................................................................

} // namespace ct
} // namespace jnc
