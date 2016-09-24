#include "pch.h"
#include "jnc_ct_DoxyParser.h"
#include "jnc_ct_DoxyLexer.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

DoxyParser::DoxyParser (Module* module)
{
	m_module = module;
	m_block = NULL;
	m_isBlockAssigned = false;
	m_isBriefDescription = false;
}

DoxyBlock*
DoxyParser::popBlock ()
{
	// only pick up unassigned non-group blocks

	DoxyBlock* doxyBlock = 
		!m_isBlockAssigned && 
		m_block &&
		m_block->getBlockKind () != DoxyBlockKind_Group ? 
		m_block : NULL;

	m_block = NULL;
	m_isBlockAssigned = false;
	m_isBriefDescription = false;

	if (!m_groupStack.isEmpty ())
	{
		GroupStackEntry entry = m_groupStack.getBack ();
		if (entry.m_namespace == m_module->m_namespaceMgr.getCurrentNamespace ())
		{
			if (!doxyBlock)
				doxyBlock = m_module->m_doxyMgr.createBlock ();
		
			if (!doxyBlock->m_group)
				doxyBlock->m_group = entry.m_group;
		}
	}

	return doxyBlock;
}

void
DoxyParser::addComment (
	const sl::StringRef& comment,
	const lex::LineCol& pos,
	bool canAppend
	)
{
	if (!m_block || !canAppend)
	{
		m_block = m_module->m_doxyMgr.createBlock ();
		m_isBriefDescription = false;
	}

	sl::String* description = m_isBriefDescription ? &m_block->m_briefDescription : &m_block->m_detailedDescription;

	DoxyLexer lexer;
	lexer.create ("doxy", comment);
	lexer.setLineCol (pos.m_line, pos.m_col + 3); // doxygen comments always start with 3 characters: ///, //!, /** /*!

	int lastTokenLine = -1;

	for (;;)
	{
		const DoxyToken* token = lexer.getToken ();
		const DoxyToken* nextToken;
		size_t i;
		
		switch (token->m_token)
		{
		case DoxyTokenKind_Error:
			m_block = NULL;
			m_isBriefDescription = NULL;
			return;

		case DoxyTokenKind_Eof:
			return;

		case DoxyTokenKind_Enum:
		case DoxyTokenKind_EnumValue:
		case DoxyTokenKind_Struct:
		case DoxyTokenKind_Union:
		case DoxyTokenKind_Class:
		case DoxyTokenKind_Alias:
		case DoxyTokenKind_Variable:
		case DoxyTokenKind_Field:
		case DoxyTokenKind_Function:
		case DoxyTokenKind_Property:
		case DoxyTokenKind_Event:
		case DoxyTokenKind_Typedef:
		case DoxyTokenKind_Namespace:
		case DoxyTokenKind_Footnote:
			nextToken = lexer.getToken (1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			if (m_isBlockAssigned) // create a new one
			{
				m_block = m_module->m_doxyMgr.createBlock ();
				m_isBriefDescription = false;
				description = m_isBriefDescription ? &m_block->m_briefDescription : &m_block->m_detailedDescription;
			}

			m_module->m_doxyMgr.setBlockTarget (m_block, (DoxyTokenKind) token->m_token, nextToken->m_data.m_string.getTrimmedString ());
			m_isBlockAssigned = true;
			lexer.nextToken ();
			break;

		case DoxyTokenKind_Group:
		case DoxyTokenKind_DefGroup:
		case DoxyTokenKind_AddToGroup:
			nextToken = lexer.getToken (1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			i = nextToken->m_data.m_string.findOneOf (" \t");
			if (i == -1)
			{
				m_block = m_module->m_doxyMgr.getGroup (nextToken->m_data.m_string);
			}
			else
			{
				DoxyGroup* group = m_module->m_doxyMgr.getGroup (sl::StringRef (nextToken->m_data.m_string, i));
				group->m_title = sl::StringRef (nextToken->m_data.m_string.cc () + i, nextToken->m_data.m_string.getLength () - i).getLeftTrimmedString ();

				m_block = group;
			}

			m_isBriefDescription = false;
			lexer.nextToken ();
			break;

		case DoxyTokenKind_InGroup:
			nextToken = lexer.getToken (1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			if (!m_block->m_group)
			{
				DoxyGroup* group = m_module->m_doxyMgr.getGroup (nextToken->m_data.m_string);
				m_block->m_group = group;
				if (m_block->getBlockKind () == DoxyBlockKind_Group)
				{
					DoxyGroup* innerGroup = (DoxyGroup*) m_block;
					innerGroup->m_parentGroupListIt = group->addGroup (innerGroup);
				}
			}

			lexer.nextToken ();
			break;

		case DoxyTokenKind_OpeningBrace:
			if (m_block->getBlockKind () == DoxyBlockKind_Group)
			{
				GroupStackEntry entry;
				entry.m_group = (DoxyGroup*) m_block;
				entry.m_namespace = m_module->m_namespaceMgr.getCurrentNamespace ();
				m_groupStack.append (entry);
			}

			break;

		case DoxyTokenKind_ClosingBrace:
			m_groupStack.pop ();
			break;

		case DoxyTokenKind_Title:
			nextToken = lexer.getToken (1);
			if (nextToken->m_token != DoxyTokenKind_Text)
				break; // ignore

			m_block->m_title = nextToken->m_data.m_string;
			lexer.nextToken ();
			break;
	
		case DoxyTokenKind_Brief:
			m_isBriefDescription = true;
			description = &m_block->m_briefDescription;

			if (!description->isEmpty ()) // user likely wants multi-paragraphed brief
				description->append ('\n');
			break;

		case DoxyTokenKind_Text:
			if (description->isEmpty ())
			{				
				description->copy (token->m_data.m_string.getLeftTrimmedString ());
				m_firstIndent = m_indent;
			}
			else
			{
				size_t indentLength = m_indent.getLength ();
				size_t firstIndentLength = m_firstIndent.getLength ();
				size_t commonIndentLength = AXL_MIN (indentLength, firstIndentLength);
				
				size_t i = 0;
				for (; i < commonIndentLength; i++)
					if (m_indent [i] != m_firstIndent [i])
						break;

				if (i < indentLength)
					description->append (m_indent.cc () + i, indentLength - i);

				description->append (token->m_data.m_string);
			}

			break;

		case '\n':
			if (lastTokenLine != token->m_pos.m_line && m_isBriefDescription && !description->isEmpty ()) // empty line ends brief description
			{
				m_isBriefDescription = false;
				description = &m_block->m_detailedDescription;
			}
			else if (!description->isEmpty ())
			{
				description->append ('\n');
			}

			m_indent = token->m_data.m_string;
			break;
		}

		lastTokenLine = token->m_pos.m_line;
		lexer.nextToken ();
	}
}

//.............................................................................

} // namespace ct
} // namespace jnc
