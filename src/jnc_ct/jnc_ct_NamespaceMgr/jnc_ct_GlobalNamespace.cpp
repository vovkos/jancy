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
#include "jnc_ct_GlobalNamespace.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ParseContext.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

void
GlobalNamespace::addBody(
	Unit* unit,
	const PragmaConfig* pragmaConfig,
	const lex::LineColOffset& pos,
	const sl::StringRef& body
) {
	if (m_body.isEmpty()) {
		m_parentUnit = unit;
		m_pragmaConfig = pragmaConfig;
		m_bodyPos = pos;
		m_body = body;
	} else {
		ExtraBody* extraBody = new ExtraBody;
		extraBody->m_unit = unit;
		extraBody->m_pragmaConfig = pragmaConfig;
		extraBody->m_pos = pos;
		extraBody->m_body = body;
		m_extraBodyList.insertTail(extraBody);
	}

	m_namespaceStatus = NamespaceStatus_ParseRequired;
}

sl::StringRef
GlobalNamespace::createLinkId() {
	return this != m_module->m_namespaceMgr.getGlobalNamespace() ?
		createLinkIdImpl(m_module) :
		sl::StringRef();
}

sl::StringRef
GlobalNamespace::createItemString(size_t index) {
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
		return this == m_module->m_namespaceMgr.getGlobalNamespace() ?
			sl::StringRef() :
			createQualifiedNameImpl(m_module);

	case ModuleItemStringKind_Synopsis:
		return this == m_module->m_namespaceMgr.getGlobalNamespace() ?
			sl::StringRef("global namespace") :
			sl::StringRef("namespace " + createQualifiedNameImpl(m_module));

	default:
		return sl::StringRef();
	}
}

sl::String
GlobalNamespace::createDoxyRefId() {
	sl::String refId;

	if (this == m_module->m_namespaceMgr.getGlobalNamespace())
		refId = JNC_GLOBAL_NAMESPACE_DOXID;
	else {
		refId = "namespace_";
		refId += getItemString(ModuleItemStringKind_QualifiedName);
		refId.replace('.', '_');
		refId.makeLowerCase();
	}

	return m_module->m_doxyModule.adjustRefId(refId);
}

bool
GlobalNamespace::parseBody() {
	sl::ConstIterator<Variable> lastVariableIt = m_module->m_variableMgr.getVariableList().getTail();
	sl::ConstIterator<Property> lastPropertyIt = m_module->m_functionMgr.getPropertyList().getTail();

	ParseContext parseContext(ParseContextKind_Body, m_module, m_parentUnit, this);

	bool result = parseBodyImpl(m_parentUnit, m_pragmaConfig, m_bodyPos, m_body);
	if (!result)
		return false;

	sl::Iterator<ExtraBody> it = m_extraBodyList.getHead();
	for (; it; it++) {
		result = parseBodyImpl(it->m_unit, it->m_pragmaConfig, it->m_pos, it->m_body);
		if (!result)
			return false;
	}

	if (m_module->getCompileState() >= ModuleCompileState_Parsed) {
		result =
			resolveOrphans() &&
			m_module->m_variableMgr.allocateNamespaceVariables(lastVariableIt) &&
			m_module->m_functionMgr.finalizeNamespaceProperties(lastPropertyIt);

		if (!result)
			return false;
	}

	m_body.clear();
	m_extraBodyList.clear();
	return true;
}

bool
GlobalNamespace::parseBodyImpl(
	Unit* unit,
	const PragmaConfig* pragmaConfig,
	const lex::LineColOffset& pos,
	const sl::StringRef& body
) {
	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(unit);

	size_t length = body.getLength();
	ASSERT(length >= 2);

	Parser parser(m_module, pragmaConfig, Parser::Mode_Parse);

	bool result = parser.parseBody(
		SymbolKind_global_declaration_list,
		lex::LineColOffset(pos.m_line, pos.m_col + 1, pos.m_offset + 1),
		body.getSubString(1, length - 2)
	);

	m_module->m_unitMgr.setCurrentUnit(prevUnit);
	return result;
}

bool
GlobalNamespace::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	const char* kind;
	sl::StringRef name;

	if (this == m_module->m_namespaceMgr.getGlobalNamespace()) {
		kind = "file";
		name = "global";
	} else {
		kind = "namespace";
		name = getItemName();
	}

	indexXml->appendFormat(
		"<compound kind='%s' refid='%s'><name>%s</name></compound>\n",
		kind,
		doxyBlock->getRefId().sz(),
		name.sz()
	);

	itemXml->format(
		"<compounddef kind='%s' id='%s' language='Jancy'>\n"
		"<compoundname>%s</compoundname>\n",
		kind,
		doxyBlock->getRefId().sz(),
		name.sz()
	);

	sl::String memberXml;
	bool result = Namespace::generateMemberDocumentation(outputDir, &memberXml, indexXml, true);
	if (!result)
		return false;

	itemXml->append(memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString();
	if (!footnoteXml.isEmpty()) {
		itemXml->append("<sectiondef>\n");
		itemXml->append(footnoteXml);
		itemXml->append("</sectiondef>\n");
	}

	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</compounddef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
