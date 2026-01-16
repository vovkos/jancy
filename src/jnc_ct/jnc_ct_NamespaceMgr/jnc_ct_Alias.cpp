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
#include "jnc_ct_Alias.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

inline
bool
failWithFindError(
	const QualifiedName& name,
	FindModuleItemResult findResult
) {
	ASSERT(!findResult.m_item);

	if (findResult.m_result)
		err::setFormatStringError("'%s' not found", name.getFullName().sz());

	return false;
}

//..............................................................................

FindModuleItemResult
Alias::finalizeFindAlias(
	FindModuleItemResult findResult,
	MemberCoord* coord
) {
	bool result = ensureResolved();
	if (!result)
		return g_errorFindModuleItemResult;

	if (coord)
		coord->append(m_targetCoord);

	findResult.m_item = m_targetItem;
	return findResult;
}

sl::StringRef
Alias::createItemString(size_t index) {
	if (index != ModuleItemStringKind_Synopsis)
		return createItemStringImpl(index, this);

	sl::String synopsis = "alias ";
	synopsis += getItemName();

	if (!m_initializer.isEmpty()) {
		synopsis += " = ";
		synopsis += getInitializerString();
	}

	return synopsis;
}

bool
Alias::resolveImpl() {
	bool result;

	ASSERT(!m_targetItem);

	if (m_flags & AliasFlag_InResolve) {
		err::setFormatStringError("can't resolve alias '%s' due to recursion", getItemName().sz());
		return false;
	}

	m_flags |= AliasFlag_InResolve;

	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Parse);

	sl::List<Token> tmpTokenList;
	cloneTokenList(&tmpTokenList, m_initializer);
	result = parser.parseTokenList(SymbolKind_qualified_name_save, &tmpTokenList);
	if (!result)
		return false;

	const QualifiedName& name = parser.getLastQualifiedName();

	// pass this as context to allow template instantiation

	MemberCoord coord;
	FindModuleItemResult findResult = m_parentNamespace->findDirectChildItemTraverse(*this, name.getFirstAtom(), &coord);
	if (!findResult.m_item)
		return failWithFindError(name, findResult);

	// for qualification names, look up in this and base types only

	enum {
		FindFlags =
			TraverseFlag_NoParentNamespace |
			TraverseFlag_NoUsingNamespaces |
			TraverseFlag_NoExtensionNamespaces
	};

	sl::ConstBoxIterator<QualifiedNameAtom> nameIt = name.getAtomList().getHead();
	for (; nameIt; nameIt++) {
		Namespace* nspace = findResult.m_item->getNamespace();
		if (nspace)
			coord.reset();
		else {
			ModuleItemKind itemKind = findResult.m_item->getItemKind();
			switch (itemKind) {
			case ModuleItemKind_Field: {
				// member operators should respect MemberCoord::m_variable and adjust opValue accordingly

				AXL_TODO("support aliases to fields within statics");
				if (coord.m_variable) {
					err::setError("fields within statics not supported yet");
					return false;
				}

				Field* field = (Field*)findResult.m_item;
				Type* fieldType = field->getType();
				DerivableType* parentType = field->getParentType();
				result = parentType->ensureLayout();
				if (!result)
					return false;

				coord.m_flags |= MemberCoordFlag_Member;
				coord.m_offset += field->getOffset();
				coord.m_llvmIndexArray.append(field->getLlvmIndex());

				if (fieldType->getTypeKind() == TypeKind_Class)
					coord.m_llvmIndexArray.append(1); // iface struct

				nspace = field->getType()->getNamespace();
				break;
				}

			case ModuleItemKind_Variable:
				coord.reset();
				coord.m_variable = (Variable*)findResult.m_item;
				nspace = ((Variable*)findResult.m_item)->getType()->getNamespace();
				break;
			}

			if (!nspace) {
				err::setFormatStringError("'%s' is not a namespace", findResult.m_item->getItemName().sz());
				return false;
			}
		}

		MemberCoord nextCoord;
		findResult = nspace->findDirectChildItemTraverse(*this, *nameIt, &nextCoord, FindFlags);
		if (!findResult.m_item)
			return failWithFindError(name, findResult);

		coord.append(nextCoord);
	}

	if (!(coord.m_flags & MemberCoordFlag_Member)) // can replace simple item
		m_parentNamespace->replaceItem(m_name, findResult.m_item);

	// otherwise, we need member coords
	m_targetItem = findResult.m_item;
	m_targetCoord = coord;
	return true;
}

bool
Alias::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format("<memberdef kind='alias' id='%s'", doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz());

	itemXml->appendFormat(
		"<initializer>= %s</initializer>\n",
		getInitializerString_xml().sz()
	);

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
