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

// warning C4065: switch statement contains 'default' but no 'case' labels

#pragma warning(disable: 4065)

namespace jnc {
namespace ct {

//..............................................................................

%%{

machine jnc;
write data;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# prepush / postpop (for fcall/fret)
#

prepush {
	stack = prePush();
}

postpop {
	postPop();
}

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# standard definitions
#

dec = [0-9];
hex = [0-9a-fA-F];
oct = [0-7];
bin = [01];
exp = [eE][+\-]?dec+;
id  = [_a-zA-Z] [_a-zA-Z0-9]*;
ws  = [ \t\r]+;
nl  = '\n' @{ newLine(p + 1); };
esc = '\\' [^\n];

lit_dq_wo_esc = '"' [^"\n\\]* ["\\]?;
lit_dq_w_esc  = '"' ([^"\n\\] | esc)* ["\\]?;
lit_sq        = "'" ([^'\n\\] | esc)* ['\\]?;
raw_lit_dq    = '"' [^"\n]* '"'?;
raw_lit_sq    = "'" [^'\n]* "'"?;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# curly-braces-enclosed body machine
#

body_main := |*

'{'          { ++m_curlyBraceLevel; };
'}'          { if (onRightCurlyBrace()) fret; };
lit_dq_w_esc ;
lit_sq       ;
'"""'        { fgoto body_lit_ml; };

'//' [^\n]*  ;
'/*' (any | nl)* :>> '*/'?
             ;
nl           ;
any          ;

*|;

body_lit_ml := |*

'"""'     { fgoto body_main; };
nl        ;
any       ;

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# multi-line literal machine
#

lit_ml := |*

(nl ws?)? '"""'
		  { createMlLiteralToken(); fgoto main; };
nl        ;
any       ;

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# formatting literal machines
#

lit_fmt := |*

esc       ;
'$!'      { createFmtLastErrorDescriptionTokens(); };
'$' id    { createFmtSimpleIdentifierTokens(); };
'$' dec+  { createFmtReGroupTokens(); };
'%' dec+  { createFmtIndexTokens(); };
'%' ([\-+ #0] dec*)? ('.' dec+)? ('l' | 'll' | 'z')? [diuxXfeEgGcsp]
          { createFmtSimpleSpecifierTokens(); };
[$%] '('  { createFmtLiteralToken(TokenKind_FmtLiteral, *ts == '%'); m_parenthesesLevelStack.append(1); fcall main; };
'"' | nl  { createFmtLiteralToken(TokenKind_Literal); fret; };
any       ;

*|;

fmt_spec := |*

';' [^")\n]* { createFmtSpecifierToken(); fret; };
any          { ASSERT(false); fret; };

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# global declarations & pragmas

'import'         { createToken(TokenKind_Import); };
'namespace'      { createToken(TokenKind_Namespace); };
'extension'      { createToken(TokenKind_Extension); };
'dynamiclib'     { createToken(TokenKind_DynamicLib); };
'using'          { createToken(TokenKind_Using); };
'friend'         { createToken(TokenKind_Friend); };
'public'         { createToken(TokenKind_Public); };
'protected'      { createToken(TokenKind_Protected); };
'pragma'         { createToken(TokenKind_Pragma); };
'setas'          { createToken(TokenKind_SetAs); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# storage specifiers

'typedef'        { createToken(TokenKind_Typedef); };
'alias'          { createToken(TokenKind_Alias); };
'static'         { createToken(TokenKind_Static); };
'threadlocal'    { createToken(TokenKind_ThreadLocal); };
'abstract'       { createToken(TokenKind_Abstract); };
'virtual'        { createToken(TokenKind_Virtual); };
'override'       { createToken(TokenKind_Override); };
'mutable'        { createToken(TokenKind_Mutable); };
'disposable'     { createToken(TokenKind_Disposable); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# type modifiers

'unsigned'       { createToken(TokenKind_Unsigned); };
'bigendian'      { createToken(TokenKind_BigEndian); };
'const'          { createToken(TokenKind_Const); };
'readonly'       { createToken(TokenKind_ReadOnly); };
'cmut'           { createToken(TokenKind_CMut); };
'volatile'       { createToken(TokenKind_Volatile); };
'weak'           { createToken(TokenKind_Weak); };
'thin'           { createToken(TokenKind_Thin); };
'safe'           { createToken(TokenKind_Safe); };
'unsafe'         { createToken(TokenKind_Unsafe); };
'errorcode'      { createToken(TokenKind_ErrorCode); };
'cdecl'          { createToken(TokenKind_Cdecl); };
'stdcall'        { createToken(TokenKind_Stdcall); };
'jnccall'        { createToken(TokenKind_Jnccall); };
'thiscall'       { createToken(TokenKind_Thiscall); };
'array'          { createToken(TokenKind_Array); };
'function'       { createToken(TokenKind_Function); };
'property'       { createToken(TokenKind_Property); };
'bindable'       { createToken(TokenKind_Bindable); };
'autoget'        { createToken(TokenKind_AutoGet); };
'indexed'        { createToken(TokenKind_Indexed); };
'multicast'      { createToken(TokenKind_Multicast); };
'event'          { createToken(TokenKind_Event); };
'reactor'        { createToken(TokenKind_Reactor); };
'async'          { createToken(TokenKind_Async); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# type specifiers

'anydata'        { createToken(TokenKind_AnyData); };
'void'           { createToken(TokenKind_Void); };
'bool'           { createToken(TokenKind_Bool); };
'int'            { createToken(TokenKind_Int); };
'intptr'         { createToken(TokenKind_IntPtr); };
'char'           { createToken(TokenKind_Char); };
'short'          { createToken(TokenKind_Short); };
'long'           { createToken(TokenKind_Long); };
'float'          { createToken(TokenKind_Float); };
'double'         { createToken(TokenKind_Double); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# named type specifiers

'enum'           { createToken(TokenKind_Enum); };
'bitflag' ws+ 'enum'
                 { createToken(TokenKind_BitFlagEnum); };
'struct'         { createToken(TokenKind_Struct); };
'union'          { createToken(TokenKind_Union); };
'class'          { createToken(TokenKind_Class); };
'opaque' ws+ 'class'
                 { createToken(TokenKind_OpaqueClass); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# special member methods

'get'            { createToken(TokenKind_Get); };
'set'            { createToken(TokenKind_Set); };
'construct'      { createToken(TokenKind_Construct); };
'static' ws+ 'construct'
                 { createToken(TokenKind_StaticConstruct); };
'destruct'       { createToken(TokenKind_Destruct); };
'operator'       { createToken(TokenKind_Operator); };
'postfix' ws+ 'operator'
                 { createToken(TokenKind_PostfixOperator); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# operators

'new'            { createToken(TokenKind_New); };
'sizeof'         { createToken(TokenKind_SizeOf); };
'countof'        { createToken(TokenKind_CountOf); };
'offsetof'       { createToken(TokenKind_OffsetOf); };
'typeof'         { createToken(TokenKind_TypeOf); };
'bindingof'      { createToken(TokenKind_BindingOf); };
'dynamic'        { createToken(TokenKind_Dynamic); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# statements

'if'             { createToken(TokenKind_If); };
'else'           { createToken(TokenKind_Else); };
'for'            { createToken(TokenKind_For); };
'while'          { createToken(TokenKind_While); };
'do'             { createToken(TokenKind_Do); };
'break'          { createKeywordTokenEx(TokenKind_Break, 1); };
'break' [1-9]    { createKeywordTokenEx(TokenKind_Break, te[-1] - '0'); };
'continue'       { createKeywordTokenEx(TokenKind_Continue, 1); };
'continue' [1-9] { createKeywordTokenEx(TokenKind_Continue, te[-1] - '0'); };
'return'         { createToken(TokenKind_Return); };
'switch'         { createToken(TokenKind_Switch); };
'lexer' ws+ 'switch'
                 { createToken(TokenKind_LexerSwitch); };
'case'           { createToken(TokenKind_Case); };
'default'        { createToken(TokenKind_Default); };
'once'           { createToken(TokenKind_Once); };
'onevent'        { createToken(TokenKind_OnEvent); };
'try'            { createToken(TokenKind_Try); };
'throw'          { createToken(TokenKind_Throw); };
'catch'          { createToken(TokenKind_Catch); };
'finally'        { createToken(TokenKind_Finally); };
'nestedscope'    { createToken(TokenKind_NestedScope); };
'assert'         { createToken(TokenKind_Assert); };
'await'          { createToken(TokenKind_Await); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# pre-defined values

'basetype'       { createKeywordTokenEx(TokenKind_BaseType, 1); };
'basetype' [1-9] { createKeywordTokenEx(TokenKind_BaseType, te[-1] - '0'); };
'this'           { createToken(TokenKind_This); };
'true'           { createToken(TokenKind_True); };
'false'          { createToken(TokenKind_False); };
'null'           { createToken(TokenKind_Null); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# symbol tokens

'++'             { createToken(TokenKind_Inc); };
'--'             { createToken(TokenKind_Dec); };
'->'             { createToken(TokenKind_Ptr); };
'=>'             { createToken(TokenKind_Imply); };
'<<'             { createToken(TokenKind_Shl); };
'>>'             { createToken(TokenKind_Shr); };
'&&'             { createToken(TokenKind_LogAnd); };
'||'             { createToken(TokenKind_LogOr); };
'=='             { createToken(TokenKind_Eq); };
'!='             { createToken(TokenKind_Ne); };
'<='             { createToken(TokenKind_Le); };
'>='             { createToken(TokenKind_Ge); };
':='             { createToken(TokenKind_RefAssign); };
'+='             { createToken(TokenKind_AddAssign); };
'-='             { createToken(TokenKind_SubAssign); };
'*='             { createToken(TokenKind_MulAssign); };
'/='             { createToken(TokenKind_DivAssign); };
'%='             { createToken(TokenKind_ModAssign); };
'<<='            { createToken(TokenKind_ShlAssign); };
'>>='            { createToken(TokenKind_ShrAssign); };
'&='             { createToken(TokenKind_AndAssign); };
'^='             { createToken(TokenKind_XorAssign); };
'|='             { createToken(TokenKind_OrAssign); };
'@='             { createToken(TokenKind_AtAssign); };
'=~'             { createToken(TokenKind_Match); };
'...'            { createToken(TokenKind_Ellipsis); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# literals

[$fF] '"'        { preCreateFmtLiteralToken(); fcall lit_fmt; };

'"""' (ws? '\r'? nl)?
				 { preCreateMlLiteralToken(); fgoto lit_ml; };

'0' [xX] '"""'   { preCreateMlLiteralToken(16); fgoto lit_ml; };
'0' [oO] '"""'   { preCreateMlLiteralToken(8); fgoto lit_ml; };
'0' [bB] '"""'   { preCreateMlLiteralToken(2); fgoto lit_ml; };
'0' [nNdD] '"""' { preCreateMlLiteralToken(10); fgoto lit_ml; };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# lexer-time constants

'__FILE__'       { createSourceFileToken(); };
'__DIR__'        { createSourceDirToken(); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# other tokens

id               { createStringToken(TokenKind_Identifier); };
lit_sq           { createCharToken(1, true); };
lit_dq_wo_esc    { createLiteralToken(1, false); };
lit_dq_w_esc     { createLiteralToken(1, true); };
[rR] raw_lit_sq  { createCharToken(2, false); };
[rR] raw_lit_dq  { createLiteralToken(2, false); };
'0' oct+         { createIntegerToken(8); };
dec+             { createIntegerToken(10); };
'0' [xX] hex+    { createIntegerToken(16, 2); };
'0' [oO] oct+    { createIntegerToken(8, 2); };
'0' [bB] bin+    { createIntegerToken(2, 2); };
'0' [nNdD] dec+  { createIntegerToken(10, 2); };

'0' [xX] raw_lit_dq
				 { createBinLiteralToken(16); };
'0' [oO] raw_lit_dq
				 { createBinLiteralToken(8); };
'0' [bB] raw_lit_dq
				 { createBinLiteralToken(2); };
'0' [nNdD] raw_lit_dq
				 { createBinLiteralToken(10); };

(dec+ '.' dec* | '.' dec+) exp? | dec+ exp
				 { createFpToken(); };

'$' dec+         { createIntegerToken(TokenKind_ReGroup, 10, 1); };

'///' [^\n]*     { createDoxyCommentToken(TokenKind_DoxyComment1); };
'//!' [^\n]*     { createDoxyCommentToken(TokenKind_DoxyComment2); };
'/**' (any | nl)* :>> '*/'?
				 { createDoxyCommentToken(TokenKind_DoxyComment3); };
'/*!' (any | nl)* :>> '*/'?
				 { createDoxyCommentToken(TokenKind_DoxyComment4); };

'//' [^\n]*      ;
'/*' (any | nl)* :>> '*/'?
                 ;
'{'              { if (onLeftCurlyBrace()) fcall body_main; };
'('              { onLeftParentheses(); };
')'              { if (!onRightParentheses()) fret; };
';'              { if (!onSemicolon()) fcall fmt_spec; };

ws | nl          ;
print            { createToken(ts[0]); };
any              { createErrorToken(ts[0]); };

*|;

}%%

//..............................................................................

void
Lexer::init() {
	%% write init;
}

void
Lexer::exec() {
	%% write exec;
}

//..............................................................................

} // namespace ct
} // namespace jnc
