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
#include "jnc_ct_DoxyParser.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

DoxyParser::DoxyParser(Module* module)
{
	m_module = module;
	m_block = NULL;
	m_parentBlock = NULL;
	m_blockTargetKind = BlockTargetKind_None;
	m_descriptionKind = DescriptionKind_Detailed;
	m_overloadIdx = 0;
}

DoxyBlock*
DoxyParser::popBlock()
{
	// only pick up unassigned non-group blocks

	DoxyBlock* doxyBlock = NULL;

	if (m_block && !m_blockTargetKind)
	{
		if (m_block->getBlockKind() == DoxyBlockKind_Footnote)
			m_block = ((DoxyFootnote*)m_block)->getParent();

		if (m_block->getBlockKind() != DoxyBlockKind_Group)
			doxyBlock = m_block;
	}

	m_block = NULL;
	m_blockTargetKind = BlockTargetKind_None;
	m_descriptionKind = DescriptionKind_Detailed;

	if (!m_groupStack.isEmpty())
	{
		GroupStackEntry entry = m_groupStack.getBack();
		if (entry.m_namespace == m_module->m_namespaceMgr.getCurrentNamespace())
		{
			if (!doxyBlock)
				doxyBlock = m_module->m_doxyMgr.createBlock();

			if (!doxyBlock->m_group)
				doxyBlock->m_group = entry.m_group;
		}
	}

	return doxyBlock;
}

void
DoxyParser::setBlockTarget(
	DoxyTokenKind tokenKind,
	const sl::StringRef& name
	)
{
	switch(tokenKind)
	{
	case DoxyTokenKind_Overload:
		if (m_overloadName == name)
		{
			m_overloadIdx++;
			break;
		}

		// else fall through

	case DoxyTokenKind_Function:
		m_overloadName = name;
		m_overloadIdx = 0;
		break;

	default:
		m_overloadName.clear();
		m_overloadIdx = 0;
	}

	m_module->m_doxyMgr.setBlockTarget(m_block, tokenKind, name, m_overloadIdx);
	m_blockTargetKind = m_block == m_parentBlock ?
		BlockTargetKind_Compound :
		BlockTargetKind_Member;
}

void
DoxyParser::addComment(
	const sl::StringRef& comment,
	const lex::LineCol& pos,
	bool canAppend,
	ModuleItem* lastDeclaredItem
	)
{
	if (!m_block || !canAppend)
	{
		m_block = m_module->m_doxyMgr.createBlock();
		m_blockTargetKind = BlockTargetKind_None;
		m_descriptionKind = DescriptionKind_Detailed;
	}

	if (lastDeclaredItem)
	{
		m_module->m_doxyMgr.setDoxyBlock(lastDeclaredItem, lastDeclaredItem->getDecl(), m_block);
		m_blockTargetKind = BlockTargetKind_Member;
	}

	sl::String* description;
	switch(m_descriptionKind)
	{
	case DescriptionKind_Brief:
		description = &m_block->m_briefDescription;
		break;

	case DescriptionKind_SeeAlso:
		description = &m_block->m_seeAlsoDescription;
		break;

	case DescriptionKind_Detailed:
	default:
		description = &m_block->m_detailedDescription;
	}

	DoxyLexer lexer;
	lexer.create("doxy", comment);
	lexer.setLineCol(pos.m_line, pos.m_col + 3); // doxygen comments always start with 3 characters: ///, //!, /** /*!

	int lastTokenLine = -1;

	for (;;)
	{
		const DoxyToken* token = lexer.getToken();
		const DoxyToken* nextToken;
		DoxyFootnote* footnote;
		size_t i;

		sl::StringRef name;
		bool isParentBlock = false;

		switch(token->m_token)
		{
		case DoxyTokenKind_Error:
			m_block = NULL;
			m_blockTargetKind = BlockTargetKind_None;
			m_descriptionKind = DescriptionKind_Detailed;
			return;

		case DoxyTokenKind_Eof:
			return;

		case DoxyTokenKind_Namespace:
		case DoxyTokenKind_Enum:
		case DoxyTokenKind_Struct:
		case DoxyTokenKind_Union:
		case DoxyTokenKind_Class:
			isParentBlock = true;
			// fall through

		case DoxyTokenKind_EnumValue:
		case DoxyTokenKind_Alias:
		case DoxyTokenKind_Variable:
		case DoxyTokenKind_Field:
		case DoxyTokenKind_Function:
		case DoxyTokenKind_Overload:
		case DoxyTokenKind_Property:
		case DoxyTokenKind_Event:
		case DoxyTokenKind_Typedef:
			nextToken = lexer.getToken(1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			if (m_blockTargetKind || m_block->getBlockKind() == DoxyBlockKind_Footnote) // create a new one
			{
				m_block = m_module->m_doxyMgr.createBlock();
				m_descriptionKind = DescriptionKind_Detailed;
				description = &m_block->m_detailedDescription;
			}

			if (isParentBlock)
				m_parentBlock = m_block;

			name = nextToken->m_data.m_string.getTrimmedString();
			setBlockTarget((DoxyTokenKind)token->m_token, name);
			lexer.nextToken();
			break;

		case DoxyTokenKind_Group:
			nextToken = lexer.getToken(1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			i = nextToken->m_data.m_string.findOneOf(" \t");
			if (i == -1)
			{
				m_block = m_module->m_doxyMgr.getGroup(nextToken->m_data.m_string);
			}
			else
			{
				DoxyGroup* group = m_module->m_doxyMgr.getGroup(nextToken->m_data.m_string.getSubString(0, i));
				group->m_title = nextToken->m_data.m_string.getSubString(i + 1).getLeftTrimmedString();

				m_block = group;
			}

			m_parentBlock = m_block;
			m_blockTargetKind = BlockTargetKind_Compound;
			m_descriptionKind = DescriptionKind_Detailed;
			description = &m_block->m_detailedDescription;
			lexer.nextToken();
			break;

		case DoxyTokenKind_InGroup:
			nextToken = lexer.getToken(1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			if (!m_block->m_group)
			{
				DoxyGroup* group = m_module->m_doxyMgr.getGroup(nextToken->m_data.m_string);
				m_block->m_group = group;
				if (m_block->getBlockKind() == DoxyBlockKind_Group)
				{
					DoxyGroup* innerGroup = (DoxyGroup*)m_block;
					innerGroup->m_parentGroupListIt = group->addGroup(innerGroup);
				}
			}

			lexer.nextToken();
			break;

		case DoxyTokenKind_OpeningBrace:
			if (m_block->getBlockKind() == DoxyBlockKind_Group)
			{
				GroupStackEntry entry;
				entry.m_group = (DoxyGroup*)m_block;
				entry.m_namespace = m_module->m_namespaceMgr.getCurrentNamespace();
				m_groupStack.append(entry);
			}

			break;

		case DoxyTokenKind_ClosingBrace:
			m_groupStack.pop();
			break;

		case DoxyTokenKind_Title:
			nextToken = lexer.getToken(1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			m_block->m_title = nextToken->m_data.m_string;
			lexer.nextToken();
			break;

		case DoxyTokenKind_Import:
			nextToken = lexer.getToken(1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			m_block->m_importList.insertTail(nextToken->m_data.m_string);
			lexer.nextToken();
			break;

		case DoxyTokenKind_SubGroup:
			m_block->m_internalDescription += ":subgroup:";
			break;

		case DoxyTokenKind_Footnote:
			nextToken = lexer.getToken(1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			if (m_block->getBlockKind() == DoxyBlockKind_Footnote)
			{
				m_block = ((DoxyFootnote*)m_block)->getParent();
				ASSERT(m_block->getBlockKind() != DoxyBlockKind_Footnote);
			}
			else if (m_blockTargetKind == BlockTargetKind_Member && m_parentBlock)
			{
				// use parent block for non-compounds (if exists)
				m_block = m_parentBlock;
			}

			name = nextToken->m_data.m_string.getTrimmedString();
			footnote = m_module->m_doxyMgr.createFootnote();
			footnote->m_refId = name;
			footnote->m_parent = m_block;
			footnote->m_parent->m_footnoteArray.append(footnote);

			m_block = footnote;
			m_descriptionKind = DescriptionKind_Detailed;
			description = &m_block->m_detailedDescription;
			lexer.nextToken();
			break;

		case DoxyTokenKind_Brief:
			m_descriptionKind = DescriptionKind_Brief;
			description = &m_block->m_briefDescription;

			if (!description->isEmpty())
				description->append('\n');

			break;

		case DoxyTokenKind_SeeAlso:
			m_descriptionKind = DescriptionKind_SeeAlso;
			description = &m_block->m_seeAlsoDescription;
			break;

		case DoxyTokenKind_OtherCommand:
			// maybe, treat it as normal text?

			break;

		case DoxyTokenKind_Text:
			if (description->isEmpty())
			{
				description->copy(token->m_data.m_string.getLeftTrimmedString());
				m_firstIndent = m_indent;
			}
			else
			{
				if (!m_indent.isEmpty())
				{
					size_t indentLength = m_indent.getLength();
					size_t firstIndentLength = m_firstIndent.getLength();
					size_t commonIndentLength = AXL_MIN(indentLength, firstIndentLength);

					size_t i = 0;
					for (; i < commonIndentLength; i++)
						if (m_indent[i] != m_firstIndent[i])
							break;

					if (i < indentLength)
						description->append(m_indent.sz() + i, indentLength - i);

					m_indent.clear(); // do this once per line
				}

				description->append(token->m_data.m_string);
			}

			break;

		case '\n':
			if (lastTokenLine != token->m_pos.m_line &&
				m_descriptionKind &&
				!description->isEmpty()) // empty line ends \brief or \seealso description
			{
				m_descriptionKind = DescriptionKind_Detailed;
				description = &m_block->m_detailedDescription;
			}
			else if (!description->isEmpty())
			{
				description->append('\n');
			}

			m_indent = token->m_data.m_string;
			break;
		}

		lastTokenLine = token->m_pos.m_line;
		lexer.nextToken();
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
