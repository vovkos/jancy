// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

//.............................................................................

enum DoxyTokenKind
{
	DoxyTokenKind_Eof = 0,
	DoxyTokenKind_Error = -1,
	DoxyTokenKind_Text = 256,

	DoxyTokenKind_Enum,
	DoxyTokenKind_Struct,
	DoxyTokenKind_Union,
	DoxyTokenKind_Class,
	DoxyTokenKind_Fn,

	DoxyTokenKind_Page,
	DoxyTokenKind_Group,
	DoxyTokenKind_Section,
	DoxyTokenKind_SubSection,
	DoxyTokenKind_SubSubSection,
	DoxyTokenKind_Par,

	DoxyTokenKind_Title,
	DoxyTokenKind_InGroup,
	DoxyTokenKind_Brief,
	DoxyTokenKind_Snippet,
	DoxyTokenKind_Image,
	DoxyTokenKind_Sa,
	DoxyTokenKind_C,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_LEX_BEGIN_TOKEN_NAME_MAP (DoxyTokenName)

	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Eof,           "eof")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Error,         "error")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Text,          "text")

	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Enum,          "\\enum")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Struct,        "\\struct")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Union,         "\\union")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Class,         "\\class")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Fn,            "\\fn")

	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Page,          "\\page")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Group,         "\\group")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Section,       "\\section")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_SubSection,    "\\subsection")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_SubSubSection, "\\subsubsection")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Par,           "\\par")

	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Title,         "\\title")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_InGroup,       "\\ingroup")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Brief,         "\\brief")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Snippet,       "\\snippet")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Image,         "\\image")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_Sa,            "\\sa")
	AXL_LEX_TOKEN_NAME (DoxyTokenKind_C,             "\\c")

AXL_LEX_END_TOKEN_NAME_MAP ();

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef lex::RagelToken <DoxyTokenKind, DoxyTokenName, lex::StdTokenData> DoxyToken;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DoxyLexer: public lex::RagelLexer <DoxyLexer, DoxyToken>
{
	friend class lex::RagelLexer <DoxyLexer, DoxyToken>;

protected:
	DoxyToken*
	createKeywordToken (DoxyTokenKind tokenKind);

	// implemented in *.rl

	void
	init ();

	void
	exec ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
