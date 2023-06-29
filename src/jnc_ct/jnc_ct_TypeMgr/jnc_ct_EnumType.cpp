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
#include "jnc_ct_EnumType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getEnumTypeFlagString(EnumTypeFlag flag) {
	static const char* stringTable[] = {
		"exposed",   // EnumTypeFlag_Exposed = 0x0010000
		"bitflag",   // EnumTypeFlag_BitFlag = 0x0020000
	};

	size_t i = sl::getLoBitIdx32(flag >> 12);

	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-enum-type-flag";
}

sl::String
getEnumTypeFlagString(uint_t flags) {
	sl::String string;

	if (flags & EnumTypeFlag_Exposed)
		string = "exposed ";

	if (flags & EnumTypeFlag_BitFlag)
		string += "bitflag ";

	if (!string.isEmpty())
		string.chop(1);

	return string;
}

//..............................................................................

bool
EnumConst::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format(
		"<enumvalue id='%s'>\n"
		"<name>%s</name>\n",
		doxyBlock->getRefId().sz(),
		m_name.sz()
	);

	if (!m_initializer.isEmpty())
		itemXml->appendFormat(
			"<initializer>= %s</initializer>\n",
			getInitializerString().sz()
		);

	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append("</enumvalue>\n");

	return true;
}

//..............................................................................

EnumType::EnumType() {
	m_typeKind = TypeKind_Enum;
	m_flags = TypeFlag_Pod;
	m_rootType = NULL;
	m_baseType = NULL;
}

bool
EnumType::isBaseType(EnumType* type) {
	if (m_baseType->getTypeKind() != TypeKind_Enum ||
		type->getRootType()->cmp(m_rootType) != 0) // fast-exit
		return false;

	EnumType* baseType = (EnumType*)m_baseType;

	for (;;) {
		if (type->cmp(baseType) == 0)
			return true;

		baseType = (EnumType*)baseType->getBaseType();
		if (baseType->getTypeKind() != TypeKind_Enum)
			return false;
	}
}

EnumConst*
EnumType::createConst(
	const sl::StringRef& name,
	sl::List<Token>* initializer
) {
	EnumConst* enumConst = new EnumConst;
	enumConst->m_module = m_module;
	enumConst->m_parentUnit = m_parentUnit;
	enumConst->m_parentEnumType = this;
	enumConst->m_name = name;

	if (initializer)
		sl::takeOver(&enumConst->m_initializer, initializer);

	m_constList.insertTail(enumConst);
	m_constArray.append(enumConst);

	bool result = addItem(enumConst);
	if (!result)
		return NULL;

	return enumConst;
}

void
EnumType::prepareSignature() {
	const char* signaturePrefix = (m_flags & EnumTypeFlag_BitFlag) ?
		(m_flags & EnumTypeFlag_Exposed) ? "EZ" : "EF" :
		(m_flags & EnumTypeFlag_Exposed) ? "EC" : "EE";

	m_signature = signaturePrefix + m_qualifiedName;
}

bool
EnumType::parseBody() {
	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	m_module->m_namespaceMgr.openNamespace(this);

	size_t length = m_body.getLength();
	ASSERT(length >= 2);

	Parser parser(m_module, m_pragmaSettings, Parser::Mode_Parse);
	bool result = parser.parseBody(
		SymbolKind_enum_const_list,
		lex::LineColOffset(m_bodyPos.m_line, m_bodyPos.m_col + 1, m_bodyPos.m_offset + 1),
		m_body.getSubString(1, length - 2)
	);

	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_unitMgr.setCurrentUnit(prevUnit);
	return true;
}

FindModuleItemResult
EnumType::findDirectChildItemTraverse(
	const sl::StringRef& name,
	MemberCoord* coord,
	uint_t flags
) {
	if (!(flags & TraverseFlag_NoThis)) {
		FindModuleItemResult findResult = findDirectChildItem(name);
		if (!findResult.m_result)
			return findResult;

		if (findResult.m_item)
			return findResult;
	}

	if (!(flags & TraverseFlag_NoBaseType)) {
		if (m_baseType->getTypeKindFlags() & TypeKindFlag_Import) {
			bool result = ((ImportType*)m_baseType)->ensureResolved();
			if (!result)
				return g_errorFindModuleItemResult;
		}

		if (m_baseType->getTypeKind() == TypeKind_Enum) {
			uint_t modFlags = (flags & ~TraverseFlag_NoThis) | TraverseFlag_NoParentNamespace;
			FindModuleItemResult findResult = ((EnumType*)m_baseType)->findDirectChildItemTraverse(name, coord, modFlags);
			if (!findResult.m_result)
				return findResult;

			if (findResult.m_item)
				return findResult;
		}
	}

	return !(flags & TraverseFlag_NoParentNamespace) && m_parentNamespace ?
		m_parentNamespace->findDirectChildItemTraverse(name, coord, flags & ~TraverseFlag_NoThis) :
		g_nullFindModuleItemResult;
}

bool
EnumType::calcLayout() {
	bool result =
		m_baseType->ensureLayout() &&
		ensureNamespaceReady() &&
		ensureAttributeValuesReady();

	if (!result)
		return false;

	Type* rootType = m_baseType;
	while (rootType->getTypeKind() == TypeKind_Enum)
		rootType = ((EnumType*)rootType)->m_baseType;

	if (rootType->getTypeKind() == TypeKind_TypedefShadow)
		rootType = ((TypedefShadowType*)rootType)->getActualType();

	m_rootType = rootType;

	if (!(m_baseType->getTypeKindFlags() & TypeKindFlag_Integer) &&
		m_baseType->getTypeKind() != TypeKind_TypedefShadow) { // typedef shadows are for documentation & code-assist
		err::setFormatStringError(
			"invalid base type %s for %s (must be integer type)",
			m_baseType->getTypeString().sz(),
			getTypeString().sz()
		);

		return false;
	}

	m_size = m_baseType->getSize();
	m_alignment = m_baseType->getAlignment();

	// assign values to consts

	EnumConst* baseConst = findBaseEnumConst();

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	m_module->m_namespaceMgr.openNamespace(this);

	result = (m_flags & EnumTypeFlag_BitFlag) ?
		calcBitflagEnumConstValues(baseConst) :
		calcEnumConstValues(baseConst);

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_unitMgr.setCurrentUnit(prevUnit);
	return result;
}

EnumConst*
EnumType::findBaseEnumConst() {
	EnumType* baseType = (EnumType*)m_baseType;
	while (baseType->getTypeKind() == TypeKind_Enum) {
		if (!baseType->m_constList.isEmpty())
			return *baseType->m_constList.getTail();

		baseType = (EnumType*)baseType->m_baseType;
	}

	return NULL;
}

bool
EnumType::calcEnumConstValues(EnumConst* baseConst) {
	bool finalResult = true;
	int64_t value = baseConst ? baseConst->m_value + 1 : 0;

	sl::Iterator<EnumConst> constIt = m_constList.getHead();
	for (; constIt; constIt++, value++) {
		bool result = constIt->ensureAttributeValuesReady();
		if (!result)
			finalResult = false;

		if (!constIt->m_initializer.isEmpty()) {
			result = m_module->m_operatorMgr.parseConstIntegerExpression(&constIt->m_initializer, &value);
			if (!result)
				finalResult = false;
		}

		constIt->m_value = value;
		constIt->m_flags |= EnumConstFlag_ValueReady;
		m_constMap[value] = *constIt;
	}

	return finalResult;
}

bool
EnumType::calcBitflagEnumConstValues(EnumConst* baseConst) {
	bool finalResult = true;
	int64_t value = baseConst ? 2 << sl::getHiBitIdx64(baseConst->m_value) : 1;

	sl::Iterator<EnumConst> constIt = m_constList.getHead();
	for (; constIt; constIt++) {
		bool result = constIt->ensureAttributeValuesReady();
		if (!result)
			finalResult = false;

		if (!constIt->m_initializer.isEmpty()) {
			result = m_module->m_operatorMgr.parseConstIntegerExpression(&constIt->m_initializer, &value);
			if (!result)
				finalResult = false;
		}

		constIt->m_value = value;
		constIt->m_flags |= EnumConstFlag_ValueReady;
		m_constMap[value] = *constIt;

		value = value ? 2 << sl::getHiBitIdx64(value) : 1;
	}

	return finalResult;
}

sl::String
EnumType::getValueString(
	const void* p,
	const char* formatSpec
) {
	int64_t n = 0;
	memcpy(&n, p, m_baseType->getSize());

	if (!(m_flags & EnumTypeFlag_BitFlag)) { // shortcut
		EnumConst* enumConst = findConst(n);
		return enumConst ? enumConst->m_name : m_baseType->getValueString(p, formatSpec);
	}

	sl::String string;

	int64_t unnamedFlags = 0;

	while (n) {
		int64_t flag = sl::getLoBit64(n);
		if (!flag)
			break;

		n &= ~flag;

		EnumConst* enumConst = findConst(flag);
		if (!enumConst) {
			unnamedFlags |= flag;
		} else {
			if (!string.isEmpty())
				string += ", ";

			string += enumConst->m_name;
		}
	}

	if (!formatSpec)
		formatSpec = "0x%X"; // bitflags are better represented as hex

	if (string.isEmpty())
		return m_baseType->getValueString(p, formatSpec);

	if (unnamedFlags) {
		string += ", ";
		string += m_baseType->getValueString(&unnamedFlags, formatSpec);
	}

	return string;
}

bool
EnumType::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	bool result = ensureNoImports();
	if (!result)
		return false;

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	sl::String memberXml;
	result = Namespace::generateMemberDocumentation(outputDir, &memberXml, indexXml, false);
	if (!result)
		return false;

	itemXml->format(
		"<memberdef kind='enum' id='%s'"
		">\n<name>%s</name>\n",
		doxyBlock->getRefId().sz(),
		m_name.sz()
	);

	if (m_baseType->getTypeKind() != TypeKind_Int)
		itemXml->append(m_baseType->getDoxyTypeString());

	uint_t flags = m_flags;
	if (m_name.isEmpty())
		flags &= ~EnumTypeFlag_Exposed; // unnamed enums imply 'exposed' anyway

	sl::String modifierString = getEnumTypeFlagString(flags);
	if (!modifierString.isEmpty())
		itemXml->appendFormat("<modifiers>%s</modifiers>\n", modifierString.sz());

	itemXml->append(memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString();
	if (!footnoteXml.isEmpty())
		itemXml->append(footnoteXml);

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
