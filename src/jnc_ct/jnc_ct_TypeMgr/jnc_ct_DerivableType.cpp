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
#include "jnc_ct_DerivableType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

BaseTypeSlot::BaseTypeSlot() {
	m_itemKind = ModuleItemKind_BaseTypeSlot;
	m_type = NULL;
	m_offset = 0;
	m_llvmIndex = -1;
	m_vtableIndex = -1;
}

//..............................................................................

BaseTypeCoord::BaseTypeCoord():
	m_llvmIndexArray(rc::BufKind_Field, m_buffer, sizeof(m_buffer)) {
	m_type = NULL;
	m_offset = 0;
	m_vtableIndex = 0;
}

//..............................................................................

DerivableType::DerivableType():
	MemberBlock(this) {
	m_operatorVararg = NULL;
	m_operatorCdeclVararg = NULL;
	m_setAsType = NULL;
}

FunctionType*
DerivableType::getMemberMethodType(
	FunctionType* shortType,
	uint_t thisArgTypeFlags
) {
	return m_module->m_typeMgr.getMemberMethodType(this, shortType, thisArgTypeFlags);
}

PropertyType*
DerivableType::getMemberPropertyType(PropertyType* shortType) {
	return m_module->m_typeMgr.getMemberPropertyType(this, shortType);
}

FindModuleItemResult
DerivableType::findItemInExtensionNamespaces(const sl::StringRef& name) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	while (nspace) {
		FindModuleItemResult findResult = nspace->getUsingSet()->findExtensionItem(this, name);
		if (!findResult.m_result || findResult.m_item)
			return findResult;

		nspace = nspace->getParentNamespace();
	}

	return g_nullFindModuleItemResult;
}

Field*
DerivableType::getFieldByIndex(size_t index) {
	if (!m_baseTypeList.isEmpty()) {
		err::setFormatStringError("'%s' has base types, cannot use indexed member operator", getTypeString().sz());
		return NULL;
	}

	size_t count = m_fieldArray.getCount();
	if (index >= count) {
		err::setFormatStringError("index '%d' is out of bounds", index);
		return NULL;
	}

	return m_fieldArray[index];
}

Property*
DerivableType::getIndexerProperty(Type* argType) {
	ASSERT(!(m_flags & ModuleItemFlag_LayoutReady));

	sl::StringHashTableIterator<Property*> it = m_indexerPropertyMap.visit(argType->getSignature());
	if (it->m_value)
		return it->m_value;

	Property* prop = m_module->m_functionMgr.createInternalProperty(createQualifiedName("m_indexer"));
	prop->m_storageKind = StorageKind_Member;
	it->m_value = prop;
	return prop;
}

Property*
DerivableType::chooseIndexerProperty(const Value& opValue) {
	CastKind bestCastKind = CastKind_None;
	Property* bestProperty = NULL;
	bool isAmbiguous = false;

	sl::StringHashTableIterator<Property*> it = m_indexerPropertyMap.getHead();
	for (; it; it++) {
		Property* prop = it->m_value;

		FunctionArg* indexArg = prop->m_getter->getType()->getArgArray() [1];
		CastKind castKind = m_module->m_operatorMgr.getCastKind(opValue, indexArg->getType());
		if (!castKind)
			continue;

		if (castKind == bestCastKind)
			isAmbiguous = true;

		if (castKind > bestCastKind) {
			bestProperty = prop;
			bestCastKind = castKind;
			isAmbiguous = false;
		}
	}

	if (!bestProperty) {
		err::setFormatStringError("none of the %d indexer properties accept the specified index argument", m_indexerPropertyMap.getCount());
		return NULL;
	}

	if (isAmbiguous) {
		err::setFormatStringError("ambiguous call to overloaded function");
		return NULL;
	}

	return bestProperty;
}

BaseTypeSlot*
DerivableType::getBaseTypeByIndex(size_t index) {
	size_t count = m_baseTypeArray.getCount();
	if (index >= count) {
		err::setFormatStringError("index '%d' is out of bounds", index);
		return NULL;
	}

	return m_baseTypeArray[index];
}

BaseTypeSlot*
DerivableType::addBaseType(Type* type) {
	BaseTypeSlot* slot = AXL_MEM_NEW(BaseTypeSlot);
	slot->m_module = m_module;
	slot->m_type = (DerivableType*)type;

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup((Type**) &slot->m_type);

	m_baseTypeList.insertTail(slot);
	m_baseTypeArray.append(slot);
	return slot;
}

size_t
DerivableType::findBaseTypeOffset(Type* type) {
	jnc::ct::BaseTypeCoord coord;
	bool result = findBaseTypeTraverse(type, &coord);
	return result ? coord.m_offset : -1;
}

bool
DerivableType::addMethod(Function* function) {
	StorageKind storageKind = function->getStorageKind();
	FunctionKind functionKind = function->getFunctionKind();
	uint_t functionKindFlags = getFunctionKindFlags(functionKind);
	uint_t thisArgTypeFlags = function->m_thisArgTypeFlags;

	function->m_parentNamespace = this;

	switch (storageKind) {
	case StorageKind_Static:
		if (thisArgTypeFlags) {
			err::setFormatStringError("static method cannot be '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
			return false;
		}

		break;

	case StorageKind_Undefined:
		function->m_storageKind = StorageKind_Member;
		// and fall through

	case StorageKind_Member:
		function->convertToMemberMethod(this);
		break;

	default:
		err::setFormatStringError("invalid storage specifier '%s' for method member", getStorageKindString(storageKind));
		return false;
	}

	Property* indexerProperty;
	sl::Array<FunctionArg*> argArray;
	Function** targetFunction = NULL;
	OverloadableFunction* targetOverloadableFunction = NULL;
	size_t overloadIdx;

	switch (functionKind) {
	case FunctionKind_StaticConstructor:
		targetFunction = &m_staticConstructor;
		break;

	case FunctionKind_Constructor:
		targetOverloadableFunction = &m_constructor;
		break;

	case FunctionKind_Normal:
		overloadIdx = addFunction(function);
		if (overloadIdx == -1)
			return false;

		m_methodArray.append(function);
		return true;

	case FunctionKind_UnaryOperator:
		if (m_unaryOperatorTable.isEmpty())
			m_unaryOperatorTable.setCountZeroConstruct(UnOpKind__Count);

		targetOverloadableFunction = &m_unaryOperatorTable[function->getUnOpKind()];
		break;

	case FunctionKind_BinaryOperator:
		if (m_binaryOperatorTable.isEmpty())
			m_binaryOperatorTable.setCountZeroConstruct(BinOpKind__Count);

		targetOverloadableFunction = &m_binaryOperatorTable[function->getBinOpKind()];
		break;

	case FunctionKind_CallOperator:
		targetOverloadableFunction = &m_callOperator;
		break;

	case FunctionKind_OperatorVararg:
		targetFunction = &m_operatorVararg;
		break;

	case FunctionKind_OperatorCdeclVararg:
		targetFunction = &m_operatorCdeclVararg;
		break;

	case FunctionKind_Getter:
		argArray = function->getType()->getArgArray();
		if (argArray.getCount() < 2) {
			err::setFormatStringError("indexer property getter should take at least one index argument");
			return false;
		}

		indexerProperty = getIndexerProperty(argArray[1]->getType());
		targetFunction = &indexerProperty->m_getter;
		break;

	case FunctionKind_Setter:
		argArray = function->getType()->getArgArray();
		if (argArray.getCount() < 3) {
			err::setFormatStringError("indexer property setter should take at least one index argument");
			return false;
		}

		indexerProperty = getIndexerProperty(argArray[1]->getType());
		targetOverloadableFunction = &indexerProperty->m_setter;
		break;

	default:
		err::setFormatStringError(
			"invalid %s in '%s'",
			getFunctionKindString(functionKind),
			getTypeString().sz()
		);
		return false;
	}

	function->m_qualifiedName = createQualifiedName(getFunctionKindString(functionKind));
	return addUnnamedMethod(function, targetFunction, targetOverloadableFunction);
}

bool
DerivableType::addProperty(Property* prop) {
	ASSERT(prop->isNamed());

	bool result = addItem(prop);
	if (!result)
		return false;

	prop->m_parentNamespace = this;

	StorageKind storageKind = prop->getStorageKind();
	switch (storageKind) {
	case StorageKind_Static:
		break;

	case StorageKind_Undefined:
		prop->m_storageKind = StorageKind_Member;
		//and fall through

	case StorageKind_Member:
		prop->m_parentType = this;
		break;

	default:
		err::setFormatStringError("invalid storage specifier '%s' for method member", getStorageKindString(storageKind));
		return false;
	}

	m_propertyArray.append(prop);
	return true;
}

bool
DerivableType::parseBody() {
	sl::ConstIterator<Variable> lastVariableIt = m_module->m_variableMgr.getVariableList().getTail();
	sl::ConstIterator<Property> lastPropertyIt = m_module->m_functionMgr.getPropertyList().getTail();

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	m_module->m_namespaceMgr.openNamespace(this);

	Parser parser(m_module, m_pragmaSettings, Parser::Mode_Parse);

	size_t length = m_body.getLength();
	ASSERT(length >= 2);

	bool result =
		parser.parseBody(
			SymbolKind_member_block_declaration_list,
			lex::LineColOffset(m_bodyPos.m_line, m_bodyPos.m_col + 1, m_bodyPos.m_offset + 1),
			m_body.getSubString(1, length - 2)
		) &&
		resolveOrphans() &&
		m_module->m_variableMgr.allocateNamespaceVariables(lastVariableIt) &&
		m_module->m_functionMgr.finalizeNamespaceProperties(lastPropertyIt);

	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_unitMgr.setCurrentUnit(prevUnit);
	return true;
}

template <typename T>
bool
ensureArrayNoImports(const sl::Array<T*>& array) {
	size_t count = array.getCount();
	for (size_t i = 0; i < count; i++) {
		bool result = array[i]->getType()->ensureNoImports();
		if (!result)
			return false;
	}

	return true;
}

template <typename T>
bool
ensureListNoImports(const sl::List<T>& list) {
	sl::ConstIterator<T> it = list.getHead();
	for (; it; it++) {
		bool result = it->getType()->ensureNoImports();
		if (!result)
			return false;
	}

	return true;
}

bool
DerivableType::resolveImports() {
	return
		ensureListNoImports(m_baseTypeList) &&
		ensureArrayNoImports(m_staticVariableArray) &&
		ensureArrayNoImports(m_fieldArray) &&
		ensureArrayNoImports(m_methodArray) &&
		ensureArrayNoImports(m_propertyArray) &&
		m_constructor.ensureNoImports() &&
		(!m_staticConstructor || m_staticConstructor->getType()->ensureNoImports()) &&
		(!m_destructor || m_destructor->getType()->ensureNoImports());
}

bool
DerivableType::callBaseTypeConstructors(const Value& thisValue) {
	bool result;

	size_t count = m_baseTypeConstructArray.getCount();
	for (size_t i = 0; i < count; i++) {
		BaseTypeSlot* slot = m_baseTypeConstructArray[i];
		if (slot->m_flags & ModuleItemFlag_Constructed) {
			slot->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		OverloadableFunction constructor = slot->m_type->getConstructor();
		ASSERT(constructor);

		result = m_module->m_operatorMgr.callOperator(constructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
DerivableType::callBaseTypeDestructors(const Value& thisValue) {
	bool result;

	size_t count = m_baseTypeDestructArray.getCount();
	for (intptr_t i = count - 1; i >= 0; i--) {
		BaseTypeSlot* slot = m_baseTypeDestructArray[i];
		Function* destructor = slot->m_type->getDestructor();
		ASSERT(destructor);

		result = m_module->m_operatorMgr.callOperator(destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
DerivableType::findBaseTypeTraverseImpl(
	Type* type,
	BaseTypeCoord* coord,
	size_t level
) {
	sl::StringHashTableIterator<BaseTypeSlot*> it = m_baseTypeMap.find(type->getSignature());
	if (it) {
		if (!coord)
			return true;

		BaseTypeSlot* slot = it->m_value;
		coord->m_type = slot->m_type;
		coord->m_offset = slot->m_offset;
		coord->m_vtableIndex = slot->m_vtableIndex;
		coord->m_llvmIndexArray.setCount(level + 1);
		coord->m_llvmIndexArray[level] = slot->m_llvmIndex;
		return true;
	}

	sl::Iterator<BaseTypeSlot> slotIt = m_baseTypeList.getHead();
	for (; slotIt; slotIt++) {
		BaseTypeSlot* slot = *slotIt;
		ASSERT(slot->m_type);

		bool result = slot->m_type->findBaseTypeTraverseImpl(type, coord, level + 1);
		if (result) {
			if (coord) {
				coord->m_offset += slot->m_offset;
				coord->m_vtableIndex += slot->m_vtableIndex;
				coord->m_llvmIndexArray[level] = slot->m_llvmIndex;
			}

			return true;
		}
	}

	return false;
}

FindModuleItemResult
DerivableType::findDirectChildItemTraverse(
	const sl::StringRef& name,
	MemberCoord* coord,
	uint_t flags,
	size_t level
) {
	if (!(flags & TraverseFlag_NoThis)) {
		FindModuleItemResult findResult = findDirectChildItem(name);
		if (!findResult.m_result)
			return findResult;

		if (findResult.m_item) {
			if (coord) {
				coord->m_type = this;
				coord->m_llvmIndexArray.setCount(level);

				if (m_typeKind == TypeKind_Union) {
					UnionCoord unionCoord;
					unionCoord.m_type = (UnionType*)this;
					unionCoord.m_level = level;
					coord->m_unionCoordArray.insert(0, unionCoord);
				}
			}

			return findResult;
		}

		uint_t modFlags = flags | TraverseFlag_NoParentNamespace;
		size_t nextLevel = level + 1;

		size_t count = m_unnamedFieldArray.getCount();
		for	(size_t i = 0; i < count; i++) {
			Field* field = m_unnamedFieldArray[i];
			if (field->getType()->getTypeKindFlags() & TypeKindFlag_Derivable) {
				DerivableType* type = (DerivableType*)field->getType();
				findResult = type->findDirectChildItemTraverse(name, coord, modFlags, nextLevel);
				if (!findResult.m_result)
					return findResult;

				if (findResult.m_item) {
					if (coord && coord->m_type) {
						coord->m_offset += field->m_offset;
						coord->m_llvmIndexArray[level] = field->m_llvmIndex;

						if (m_typeKind == TypeKind_Union) {
							UnionCoord unionCoord;
							unionCoord.m_type = (UnionType*)this;
							unionCoord.m_level = level;
							coord->m_unionCoordArray.insert(0, unionCoord);
						}
					}

					return findResult;
				}
			}
		}
	}

	if (!(flags & TraverseFlag_NoExtensionNamespaces)) {
		FindModuleItemResult findResult = findItemInExtensionNamespaces(name);
		if (!findResult.m_result || findResult.m_item)
			return findResult;
	}

	if (!(flags & TraverseFlag_NoBaseType)) {
		uint_t modFlags = (flags & ~TraverseFlag_NoThis) | TraverseFlag_NoParentNamespace;
		size_t nextLevel = level + 1;

		sl::Iterator<BaseTypeSlot> slotIt = m_baseTypeList.getHead();
		for (; slotIt; slotIt++) {
			BaseTypeSlot* slot = *slotIt;
			if (slot->m_type->getTypeKindFlags() & TypeKindFlag_Import) // unresolved yet
				continue;

			DerivableType* baseType = slot->m_type;
			FindModuleItemResult findResult = baseType->findDirectChildItemTraverse(name, coord, modFlags, nextLevel);
			if (!findResult.m_result)
				return findResult;

			if (findResult.m_item) {
				if (coord && coord->m_type) {
					coord->m_offset += slot->m_offset;
					coord->m_llvmIndexArray[level] = slot->m_llvmIndex;
					coord->m_vtableIndex += slot->m_vtableIndex;
				}

				return findResult;
			}
		}
	}

	return !(flags & TraverseFlag_NoParentNamespace) && m_parentNamespace ?
		m_parentNamespace->findDirectChildItemTraverse(name, coord, flags & ~TraverseFlag_NoThis) :
		g_nullFindModuleItemResult;
}

sl::String
DerivableType::getValueString(
	const void* p0,
	const char* formatSpec
) {
	if (m_fieldArray.isEmpty())
		return "{}";

	const char* p = (const char*)p0;

	sl::String string = "{ " + m_fieldArray[0]->getType()->getValueString(p + m_fieldArray[0]->getOffset(), formatSpec);

	size_t count = m_fieldArray.getCount();
	for (size_t i = 1; i < count; i++) {
		string += ", ";
		string += m_fieldArray[i]->getType()->getValueString(p + m_fieldArray[i]->getOffset(), formatSpec);
	}

	string += " }";
	return string;
}

bool
DerivableType::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	bool result = ensureNoImports();
	if (!result)
		return false;

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	const char* kind =
		m_typeKind == TypeKind_Struct ? "struct" :
		m_typeKind == TypeKind_Union ? "union" : "class";

	indexXml->appendFormat(
		"<compound kind='%s' refid='%s'><name>%s</name></compound>\n",
		kind,
		doxyBlock->getRefId().sz(),
		getQualifiedName().sz()
	);

	sl::String constructorXml;
	sl::String destructorXml;
	if (m_constructor) {
		result = m_constructor->generateDocumentation(outputDir, &constructorXml, indexXml);
		if (!result)
			return false;
	}

	if (m_destructor) {
		result = m_destructor->generateDocumentation(outputDir, &destructorXml, indexXml);
		if (!result)
			return false;
	}

	sl::String memberXml;
	result = Namespace::generateMemberDocumentation(outputDir, &memberXml, indexXml, true);
	if (!result)
		return false;

	itemXml->format(
		"<compounddef kind='%s' id='%s' language='Jancy'>\n"
		"<compoundname>%s</compoundname>\n\n",
		kind,
		doxyBlock->getRefId().sz(),
		m_name.sz()
	);

	sl::Iterator<BaseTypeSlot> it = m_baseTypeList.getHead();
	for (; it; it++) {
		DerivableType* baseType = it->getType();
		dox::Block* baseTypeDoxyBlock = m_module->m_doxyHost.getItemBlock(baseType);
		sl::String refId = baseTypeDoxyBlock->getRefId();
		Unit* unit = baseType->getParentUnit();
		ExtensionLib* lib = unit ? unit->getLib() : NULL;
		if (lib)
			itemXml->appendFormat("<basecompoundref importid='%s/%s'>", lib->m_guid->getString().sz(), refId.sz());
		else
			itemXml->appendFormat("<basecompoundref refid='%s'>", refId.sz());

		itemXml->appendFormat("%s</basecompoundref>\n", baseType->getQualifiedName().sz());
	}

	if (!constructorXml.isEmpty() || !destructorXml.isEmpty()) {
		itemXml->append("<sectiondef>\n");
		itemXml->append(constructorXml);
		itemXml->append(destructorXml);
		itemXml->append("</sectiondef>\n\n");
	}

	itemXml->append(memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString();
	if (!footnoteXml.isEmpty()) {
		itemXml->append("<sectiondef>\n");
		itemXml->append(footnoteXml);
		itemXml->append("</sectiondef>\n");
	}

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</compounddef>\n");

	return true;
}

bool
DerivableType::requireConstructor() {
	if (!m_constructor)
		return true;

	if (m_constructor->getItemKind() == ModuleItemKind_Function) {
		Function* constructor = m_constructor.getFunction();

		if (constructor->canCompile())
			m_module->markForCompile(constructor);
	} else {
		FunctionOverload* constructor = m_constructor.getFunctionOverload();
		size_t count = constructor->getOverloadCount();
		for (size_t i = 0; i < count; i++) {
			Function* overload = constructor->getOverload(i);
			if (overload->canCompile())
				m_module->markForCompile(overload);
		}
	}

	return true;
}

bool
DerivableType::compileDefaultStaticConstructor() {
	ASSERT(m_staticConstructor);

	m_module->m_namespaceMgr.openNamespace(this);
	m_module->m_functionMgr.internalPrologue(m_staticConstructor);

	primeStaticVariables();

	bool result =
		initializeStaticVariables() &&
		callPropertyStaticConstructors();

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	m_module->m_namespaceMgr.closeNamespace();
	return true;
}

bool
DerivableType::compileDefaultConstructor() {
	Function* constructor = m_constructor.getFunction();

	Value thisValue;
	m_module->m_namespaceMgr.openNamespace(this);
	m_module->m_functionMgr.internalPrologue(constructor, &thisValue, 1);

	bool result =
		callBaseTypeConstructors(thisValue) &&
		callStaticConstructor() &&
		initializeFields(thisValue) &&
		callPropertyConstructors(thisValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	m_module->m_namespaceMgr.closeNamespace();
	return true;
}

bool
DerivableType::compileDefaultDestructor() {
	ASSERT(m_destructor);

	bool result;

	Value argValue;
	m_module->m_functionMgr.internalPrologue(m_destructor, &argValue, 1);

	result =
		callPropertyDestructors(argValue) &&
		callBaseTypeDestructors(argValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
