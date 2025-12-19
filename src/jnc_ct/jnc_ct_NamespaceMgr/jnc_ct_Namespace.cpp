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
#include "jnc_ct_Namespace.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_ClassType.h"
#include "jnc_ct_FunctionOverload.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_DynamicLibNamespace.h"

namespace jnc {
namespace ct {

//..............................................................................

void
Namespace::clear() {
	m_parentUnit = NULL;
	m_body.clear();
	m_bodyPos.clear();
	m_itemArray.clear();
	m_orphanArray.clear();
	m_itemMap.clear();
	m_friendSet.clear();
	m_dualPtrTypeTupleMap.clear();
	m_usingSet.clear();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
addOrphanFail(
	ModuleItem* item,
	Orphan* orphan
) {
	err::setFormatStringError(
		"'%s' is a %s, not a namespace",
		item->getItemName().sz(),
		getModuleItemKindString(item->getItemKind())
	);

	orphan->pushSrcPosError();
	return false;
}

bool
addOrphanToNamespace(
	ModuleItem* item,
	Orphan* orphan
) {
	((GlobalNamespace*)item)->addOrphan(orphan);
	return true;
}

bool
addOrphanToType(
	ModuleItem* item,
	Orphan* orphan
) {
	if (!(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named))
		return addOrphanFail(item, orphan);

	((NamedType*)item)->addOrphan(orphan);
	return true;
}

bool
addOrphanToTypedef(
	ModuleItem* item,
	Orphan* orphan
) {
	return addOrphanToType(((Typedef*)item)->getType(), orphan);
}

bool
addOrphanToProperty(
	ModuleItem* item,
	Orphan* orphan
) {
	((Property*)item)->addOrphan(orphan);
	return true;
}

bool
addOrphanToPropertyTemplate(
	ModuleItem* item,
	Orphan* orphan
) {
	((PropertyTemplate*)item)->addOrphan(orphan);
	return true;
}

bool
addOrphanToTemplate(
	ModuleItem* item,
	Orphan* orphan
) {
	((Template*)item)->addOrphan(orphan);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Namespace::resolveOrphans() {
	if (m_orphanArray.isEmpty())
		return true;

	typedef
	bool
	AdoptOrphanFunc(
		ModuleItem* item,
		Orphan*	orphan
	);

	static AdoptOrphanFunc* addOrphanFuncTable[ModuleItemKind__Count] = {
		addOrphanFail,               // ModuleItemKind_Undefined = 0,
		addOrphanToNamespace,        // ModuleItemKind_Namespace,
		addOrphanFail,               // ModuleItemKind_Scope,
		addOrphanFail,               // ModuleItemKind_Attribute,
		addOrphanFail,               // ModuleItemKind_AttributeBlock,
		addOrphanToType,             // ModuleItemKind_Type,
		addOrphanToTypedef,          // ModuleItemKind_Typedef,
		addOrphanFail,               // ModuleItemKind_Alias,
		addOrphanFail,               // ModuleItemKind_Const,
		addOrphanFail,               // ModuleItemKind_Variable,
		addOrphanFail,               // ModuleItemKind_Function,
		addOrphanFail,               // ModuleItemKind_FunctionArg,
		addOrphanFail,               // ModuleItemKind_FunctionOverload,
		addOrphanToProperty,         // ModuleItemKind_Property,
		addOrphanToPropertyTemplate, // ModuleItemKind_PropertyTemplate,
		addOrphanFail,               // ModuleItemKind_EnumConst,
		addOrphanFail,               // ModuleItemKind_Field,
		addOrphanFail,               // ModuleItemKind_BaseTypeSlot,
		addOrphanFail,               // ModuleItemKind_Orphan,
		addOrphanFail,               // ModuleItemKind_LazyImport,
		addOrphanFail,               // ModuleItemKind_DynamicSection,
		addOrphanToTemplate,         // ModuleItemKind_Template,
	};

	bool result;
	char buffer[256];
	sl::Array<Property*> propertyArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	ModuleItem* nspaceItem = getDeclItem();

	size_t count = m_orphanArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Orphan* orphan = m_orphanArray[i];
		FunctionKind functionKind = orphan->getFunctionKind();

		if (functionKind != FunctionKind_Normal && !orphan->m_declaratorNamePos) {
			result = orphan->adopt(nspaceItem);
			if (!result) {
				orphan->pushSrcPosError();
				return false;
			}

			continue;
		}

		const QualifiedNameAtom& atom = orphan->m_declaratorNamePos.next(orphan->m_declaratorName);
		FindModuleItemResult findResult = findDirectChildItem(atom);
		if (!findResult.m_result) {
			orphan->pushSrcPosError();
			return false;
		}

		if (!findResult.m_item) {
			err::setFormatStringError("'%s' not part of '%s'", atom.getString().sz(), getDeclItem()->getItemName().sz());
			orphan->pushSrcPosError();
			return false;
		}

		if (functionKind == FunctionKind_Normal && !orphan->m_declaratorNamePos) {
			result = orphan->adopt(findResult.m_item);
			if (!result) {
				orphan->pushSrcPosError();
				return false;
			}

			continue;
		}

		ModuleItemKind itemKind = findResult.m_item->getItemKind();
		ASSERT((size_t)itemKind < countof(addOrphanFuncTable));
		result = addOrphanFuncTable[(size_t)itemKind](findResult.m_item, orphan);
		if (!result) {
			orphan->pushSrcPosError();
			return false;
		}

		if (itemKind == ModuleItemKind_Property)
			propertyArray.append((Property*)findResult.m_item);
	}

	count = propertyArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = propertyArray[i]->resolveOrphans();
		if (!result)
			return false;
	}

	m_orphanArray.clear(); // not needed anymore
	return true;
}

bool
Namespace::ensureNamespaceReady() {
	bool result;
	switch (m_namespaceStatus) {
	case NamespaceStatus_ParseRequired:
		ASSERT(hasBody());

		m_namespaceStatus = NamespaceStatus_Parsing; // prevent recursive parse

		result = parseBody();
		if (!result) {
			m_namespaceStatus = NamespaceStatus_ParseError;
			m_parseError = err::getLastError();
			return false;
		}

		m_namespaceStatus = NamespaceStatus_Ready;
		break;

	case NamespaceStatus_ParseError:
		err::setError(m_parseError);
		return false;

	default:
		// it's OK if we are still parsing ourselves

		ASSERT(
			m_namespaceStatus == NamespaceStatus_Parsing ||
			m_namespaceStatus == NamespaceStatus_Ready);
	}

	return true;
}

bool
Namespace::ensureNamespaceReadyDeep() {
	bool result = ensureNamespaceReady();
	if (!result)
		return false;

	size_t count = m_itemArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Namespace* nspace = m_itemArray[i]->getNamespace();
		if (nspace) {
			result = nspace->ensureNamespaceReadyDeep();
			if (!result)
				return false;
		}
	}

	return true;
}

bool
Namespace::parseLazyImports() {
	sl::StringHashTableIterator<ModuleItem*> it = m_itemMap.getHead();
	for (; it; it++)
		if (it->m_value->getItemKind() == ModuleItemKind_LazyImport) {
			LazyImport* lazyImport = (LazyImport*)it->m_value;
			Module* module = lazyImport->getModule();
			module->m_importMgr.parseLazyImport(lazyImport);
		}

	if (hasBody())
		m_namespaceStatus = NamespaceStatus_ParseRequired;

	ensureNamespaceReady();
	return true;
}

FindModuleItemResult
Namespace::findDirectChildItem(const sl::StringRef& name) {
	if (m_namespaceKind == NamespaceKind_Type && name == m_name) { // shortcut to selftype
		NamedType* type = (NamedType*)this;
		if (!(type->getTypeKindFlags() & TypeKindFlag_Derivable))
			return FindModuleItemResult(type);

		TemplateInstance* templateInstance = ((DerivableType*)type)->getTemplateInstance();
		return templateInstance ?
			FindModuleItemResult(templateInstance->m_template) :
			FindModuleItemResult(type);
	}

	bool result = ensureNamespaceReady();
	if (!result)
		return g_errorFindModuleItemResult;

	sl::StringHashTableIterator<ModuleItem*> it = m_itemMap.find(name);
	if (!it)
		return g_nullFindModuleItemResult;

	ModuleItem* item = it->m_value;
	if (!item)
		return g_nullFindModuleItemResult;

	if (item->getItemKind() == ModuleItemKind_Alias) {
		Alias* alias = (Alias*)item;
		result = alias->ensureResolved();
		if (!result)
			return g_errorFindModuleItemResult;

		item = alias->getTargetItem();
	}

	if (item->getItemKind() != ModuleItemKind_LazyImport)
		return FindModuleItemResult(item);

	Module* module = item->getModule();
	ASSERT(this == module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc));

	if (module->getCompileState() >= ModuleCompileState_Compiled ||
		(((LazyImport*)item)->getFlags() & LazyImportFlag_Used))
		return g_nullFindModuleItemResult;

	m_namespaceStatus = NamespaceStatus_ParseRequired;

	// during code-assist, we may fail on parsing imports, but we still need to return a valid stdtype

	bool result1 = module->m_importMgr.parseLazyImport((LazyImport*)item);
	bool result2 = ensureNamespaceReady();

	if (!result1 || !result2)
		return it->m_value != item ?
			FindModuleItemResult(it->m_value) :
			g_errorFindModuleItemResult;

	ASSERT(it->m_value != item); // should have been replaced
	return FindModuleItemResult(it->m_value);
}

FindModuleItemResult
Namespace::findDirectChildItem(
	const ModuleItemContext& context,
	const QualifiedNameAtom& atom
) {
	Template* templ;
	switch (atom.m_atomKind) {
	case QualifiedNameAtomKind_BaseType:
		err::setError("finding basetypes is not implemented yet");
		return g_errorFindModuleItemResult;

	case QualifiedNameAtomKind_Name:
		return findDirectChildItem(atom.m_name);

	case QualifiedNameAtomKind_TemplateDeclSuffix:
	case QualifiedNameAtomKind_TemplateInstantiateOperator:
		return finalizeFindTemplate(context, atom, findDirectChildItem(atom.m_name));

	default:
		ASSERT(false);
		return g_nullFindModuleItemResult;
	}
}

FindModuleItemResult
Namespace::findItem(
	const ModuleItemContext& context,
	const QualifiedName& name
) {
	FindModuleItemResult findResult = findDirectChildItem(context, name.getFirstAtom());
	if (!findResult.m_item)
		return findResult;

	sl::ConstBoxIterator<QualifiedNameAtom> it = name.getAtomList().getHead();
	for (; it; it++) {
		Namespace* nspace = findResult.m_item->getNamespace();
		if (!nspace)
			return g_nullFindModuleItemResult;

		findResult = nspace->findDirectChildItem(context, *it);
		if (!findResult.m_item)
			return findResult;
	}

	return findResult;
}

FindModuleItemResult
Namespace::findItemTraverse(
	const ModuleItemContext& context,
	const QualifiedName& name,
	MemberCoord* coord,
	uint_t flags
) {
	FindModuleItemResult findResult = findDirectChildItemTraverse(context, name.getFirstAtom(), coord, flags);
	if (!findResult.m_item)
		return findResult;

	sl::ConstBoxIterator<QualifiedNameAtom> nameIt = name.getAtomList().getHead();
	for (; nameIt; nameIt++) {
		Namespace* nspace = findResult.m_item->getNamespace();
		if (!nspace)
			return g_nullFindModuleItemResult;

		findResult = nspace->findDirectChildItem(context, *nameIt);
		if (!findResult.m_item)
			return findResult;
	}

	return findResult;
}

FindModuleItemResult
Namespace::findDirectChildItemTraverse(
	const sl::StringRef& name,
	MemberCoord* coord,
	uint_t flags
) {
	if (!(flags & TraverseFlag_NoThis)) {
		FindModuleItemResult findResult = findDirectChildItem(name);
		if (!findResult.m_result || findResult.m_item)
			return findResult;
	}

	if (!(flags & TraverseFlag_NoUsingNamespaces)) {
		FindModuleItemResult findResult = m_usingSet.findItem(name);
		if (!findResult.m_result)
			return findResult;

		if (findResult.m_item) {
			findResult.m_result = findResult.m_item->getModule()->m_operatorMgr.checkAccess(findResult.m_item->getDecl());
			return findResult;
		}
	}

	return !(flags & TraverseFlag_NoParentNamespace) && m_parentNamespace ?
		m_parentNamespace->findDirectChildItemTraverse(name, coord, flags & ~TraverseFlag_NoThis) :
		g_nullFindModuleItemResult;
}

FindModuleItemResult
Namespace::findDirectChildItemTraverse(
	const ModuleItemContext& context,
	const QualifiedNameAtom& atom,
	MemberCoord* coord,
	uint_t flags
) {
	Template* templ;
	switch (atom.m_atomKind) {
	case QualifiedNameAtomKind_BaseType:
		err::setError("finding basetypes is not implemented yet");
		return g_errorFindModuleItemResult;

	case QualifiedNameAtomKind_Name:
		return findDirectChildItemTraverse(atom.m_name, coord, flags);

	case QualifiedNameAtomKind_TemplateDeclSuffix:
	case QualifiedNameAtomKind_TemplateInstantiateOperator:
		if (context.isNullContext()) {
			err::setError("templates are not allowed here");
			return g_errorFindModuleItemResult;
		}

		return finalizeFindTemplate(context, atom, findDirectChildItemTraverse(atom.m_name, coord, flags));

	default:
		ASSERT(false);
		return g_nullFindModuleItemResult;
	}

}

FindModuleItemResult
Namespace::finalizeFindTemplate(
	const ModuleItemContext& context,
	const QualifiedNameAtom& atom,
	FindModuleItemResult findResult
) {
	if (!findResult.m_item)
		return findResult;

	Template* templ;
	ModuleItemKind itemKind = findResult.m_item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Template:
		templ = ((Template*)findResult.m_item);
		break;

	case ModuleItemKind_Typedef:
		Namespace* nspace;
		if (((Typedef*)findResult.m_item)->isRecursive() &&
			(nspace = ((Typedef*)findResult.m_item)->getGrandParentNamespace())
		) {
			findResult = nspace->findDirectChildItemTraverse(atom.m_name);
			if (!findResult.m_item)
				return findResult;

			if (findResult.m_item->getItemKind() == ModuleItemKind_Template) {
				templ = ((Template*)findResult.m_item);
				break;
			}

			break;
		}

		// else fall through

	default:
		err::setFormatStringError("'%s' is not a template", findResult.m_item->getItemName().sz());
		return g_errorFindModuleItemResult;
	}

	switch (atom.m_atomKind) {
	case QualifiedNameAtomKind_TemplateDeclSuffix: {
		const sl::Array<TemplateArgType*> argArray = templ->getArgArray();
		size_t count = argArray.getCount();
		if (atom.m_templateDeclArgArray.getCount() != count) {
			err::setFormatStringError("template signature mismatch for '%s'", findResult.m_item->getItemName().sz());
			return g_errorFindModuleItemResult;
		}

		for (size_t i = 0; i < count; i++)
			if (atom.m_templateDeclArgArray[i]->getName() != argArray[i]->getName()) {
				err::setFormatStringError("template signature mismatch for '%s'", findResult.m_item->getItemName().sz());
				return g_errorFindModuleItemResult;
			}

		break;
		}

	case QualifiedNameAtomKind_TemplateInstantiateOperator: {
		ModuleItem* item = templ->instantiate(context, atom.m_templateInstTokenList);
		findResult = item ? FindModuleItemResult(item) : g_errorFindModuleItemResult;
		break;
		}

	default:
		ASSERT(false);
	}

	return findResult;
}

NamedType*
Namespace::findTemplateInstanceType(Template* templ) {
	for (Namespace* nspace = this; nspace; nspace = nspace->getParentNamespace()) {
		if (nspace->m_namespaceKind != NamespaceKind_Type)
			continue;

		NamedType* type = (NamedType*)nspace;
		TemplateInstance* templateInstance = type->getTemplateInstance();
		if (templateInstance && templateInstance->m_template == templ)
			return type;
	}

	return NULL;
}

bool
Namespace::addItem(
	const sl::StringRef& name,
	ModuleItem* item
) {
	sl::StringHashTableIterator<ModuleItem*> it = m_itemMap.visit(name);
	if (it->m_value && it->m_value->getItemKind() != ModuleItemKind_LazyImport) {
		setRedefinitionError(name);
		return false;
	}

	if (item->getItemKind() != ModuleItemKind_LazyImport)
		m_itemArray.append(item);

	it->m_value = item;
	return true;
}

size_t
Namespace::addFunction(Function* function) {
	size_t result = 0;

	sl::StringHashTableIterator<ModuleItem*> it = m_itemMap.visit(function->m_name);
	if (!it->m_value) {
		it->m_value = function;
	} else {
		ModuleItemKind itemKind = it->m_value->getItemKind();
		switch (itemKind) {
		case ModuleItemKind_Function:
			if (((Function*)it->m_value)->isPrototype()) { // replace the prototype
				it->m_value = function;
				break;
			}

			it->m_value = function->getModule()->m_functionMgr.createFunctionOverload((Function*)it->m_value);
			// and fall through

		case ModuleItemKind_FunctionOverload:
			result = ((FunctionOverload*)it->m_value)->addOverload(function);
			if (result == -1)
				return -1;
			break;

		case ModuleItemKind_LazyImport:
			it->m_value = function;
			break;

		default:
			setRedefinitionError(function->m_name);
			return -1;
		}
	}

	m_itemArray.append(function);
	return result;
}

Const*
Namespace::createConst(
	const sl::StringRef& name,
	const Value& value
) {
	ASSERT(value.getType());

	Module* module = value.getType()->getModule();
	Const* cnst = module->m_constMgr.createConst(name, value);
	bool result = addItem(cnst);
	if (!result)
		return NULL;

	return cnst;
}

bool
Namespace::exposeEnumConsts(EnumType* type) {
	bool result = type->ensureNamespaceReady();
	if (!result)
		return false;

	sl::Array<EnumConst*> constArray = type->getConstArray();
	size_t count = constArray.getCount();
	for (size_t i = 0; i < count; i++) {
		EnumConst* enumConst = constArray[i];
		result = addItem(enumConst);
		if (!result)
			return false;
	}

	return true;
}

bool
Namespace::generateMemberDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml,
	bool useSectionDef
) {
	bool result;

	static char compoundFileHdr[] =
		"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
		"<doxygen>\n";

	static char compoundFileTerm[] = "</doxygen>\n";

	itemXml->clear();

	sl::String sectionDef;
	sl::String memberXml;

	sl::Array<EnumType*> unnamedEnumTypeArray;

	size_t count = m_itemArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ModuleItem* item = m_itemArray[i];
		ModuleItemDecl* decl = item->getDecl();
		if (!decl)
			continue;

		Unit* unit = decl->getParentUnit();
		if (!unit || unit->getLib()) // don't document imported libraries
			continue;

		Namespace* itemNamespace = item->getNamespace();
		if (itemNamespace == this)
			continue;

		if (item->getItemKind() == ModuleItemKind_EnumConst) {
			EnumType* enumType = ((EnumConst*)item)->getType();
			if (enumType != this) { // otherwise, we are being called on behalf of EnumType
				if (!enumType->getName().isEmpty()) // exposed enum, not unnamed
					continue;

				unnamedEnumTypeArray.append(enumType);

				// skip all other enum consts of this unnamed enum

				for (i++; i < count; i++) {
					item = m_itemArray[i];
					if (item->getItemKind() != ModuleItemKind_EnumConst ||
						((EnumConst*)item)->getType() != enumType)
						break;
				}

				i--; // undo last increment
				continue;
			}
		}

		memberXml.clear();
		result = item->generateDocumentation(outputDir, &memberXml, indexXml);
		if (!result)
			return false;

		if (memberXml.isEmpty())
			continue;

		Module* module = item->getModule();
		dox::Block* doxyBlock = module->m_doxyHost.getItemBlock(item, decl);
		dox::Group* doxyGroup = doxyBlock->getGroup();
		if (doxyGroup)
			doxyGroup->addItem(item);

		ModuleItemKind itemKind = item->getItemKind();
		bool isCompoundFile =
			itemKind == ModuleItemKind_Namespace ||
			itemKind == ModuleItemKind_Type && ((Type*)item)->getTypeKind() != TypeKind_Enum;

		if (!isCompoundFile) {
			sectionDef.append(memberXml);
			sectionDef.append('\n');
		} else {
			sl::String refId = doxyBlock->getRefId();
			sl::String fileName = sl::String(outputDir) + "/" + refId + ".xml";

			io::File compoundFile;
			result =
				compoundFile.open(fileName, io::FileFlag_Clear) &&
				compoundFile.write(compoundFileHdr, lengthof(compoundFileHdr)) != -1 &&
				compoundFile.write(memberXml, memberXml.getLength()) != -1 &&
				compoundFile.write(compoundFileTerm, lengthof(compoundFileTerm)) != -1;

			if (!result)
				return false;

			const char* elemName = itemKind == ModuleItemKind_Namespace ? "innernamespace" : "innerclass";
			itemXml->appendFormat("<%s refid='%s'/>", elemName, refId.sz());
			itemXml->append('\n');
		}
	}

	count = unnamedEnumTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ModuleItem* item = unnamedEnumTypeArray[i];
		result = item->generateDocumentation(outputDir, &memberXml, indexXml);
		if (!result)
			return false;

		ASSERT(!memberXml.isEmpty());
		sectionDef.append(memberXml);
		sectionDef.append('\n');
	}

	if (!sectionDef.isEmpty())
		if (!useSectionDef) {
			itemXml->append(sectionDef);
		} else {
			itemXml->append("<sectiondef>\n");
			itemXml->append(sectionDef);
			itemXml->append("</sectiondef>\n");
		}

	return true;
}

//..............................................................................

sl::StringRef
TemplateNamespace::createItemString(size_t index) {
	switch (index) {
	case ModuleItemStringKind_Synopsis:
		return sl::StringRef("template namespace");

	default:
		return sl::StringRef();
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
