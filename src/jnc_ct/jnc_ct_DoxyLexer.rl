// warning C4065: switch statement contains 'default' but no 'case' labels

#pragma warning (disable: 4065)

namespace jnc {
namespace ct {

//.............................................................................

%%{

machine dox;
write data;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

'\\enum'          { createKeywordToken (DoxyTokenKind_Enum); };
'\\struct'        { createKeywordToken (DoxyTokenKind_Struct); };
'\\union'         { createKeywordToken (DoxyTokenKind_Union); };
'\\class'         { createKeywordToken (DoxyTokenKind_Class); };
'\\fn'            { createKeywordToken (DoxyTokenKind_Fn); };

'\\page'          { createKeywordToken (DoxyTokenKind_Page); };
'\\group'         { createKeywordToken (DoxyTokenKind_Group); };
'\\section'       { createKeywordToken (DoxyTokenKind_Section); };
'\\subsection'    { createKeywordToken (DoxyTokenKind_SubSection); };
'\\subsubsection' { createKeywordToken (DoxyTokenKind_SubSubSection); };
'\\par'           { createKeywordToken (DoxyTokenKind_Par); };

'\\title'         { createKeywordToken (DoxyTokenKind_Title); };
'\\ingroup'       { createKeywordToken (DoxyTokenKind_InGroup); };
'\\brief'         { createKeywordToken (DoxyTokenKind_Brief); };
'\\snippet'       { createKeywordToken (DoxyTokenKind_Snippet); };
'\\image'         { createKeywordToken (DoxyTokenKind_Image); };
'\\sa'            { createKeywordToken (DoxyTokenKind_Sa); };
'\\c'             { createKeywordToken (DoxyTokenKind_C); };

any               ;

*|;

}%%

//.............................................................................

void
DoxyLexer::init ()
{
	%% write init;
}

void
DoxyLexer::exec ()
{
	%% write exec;
}

//.............................................................................

} // namespace ct
} // namespace jnc
