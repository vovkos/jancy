// warning C4065: switch statement contains 'default' but no 'case' labels

#pragma warning (disable: 4065)

namespace jnc {

//.............................................................................

%%{

machine jnc;
write data;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# prepush / postpop (for fcall/fret)
#

prepush
{
	stack = prePush ();
}

postpop
{
	postPop ();
}

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# standard definitions
#

dec    = [0-9];
hex    = [0-9a-fA-F];
oct    = [0-7];
bin    = [01];
id     = [_a-zA-Z] [_a-zA-Z0-9]*;
ws     = [ \t\r]+;
nl     = '\n' @{ newLine (p + 1); };
esc    = '\\' [^\n];
lit_dq = '"' ([^"\n\\] | esc)* (["\\] | nl);
lit_sq = "'" ([^'\n\\] | esc)* (['\\] | nl);

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# formatting literal machines
#

lit_fmt := |*

esc       ;
'$' id    { createFmtSimpleIdentifierToken (); };
'%' dec+  { createFmtIndexToken (); };
[$%] '('  { createFmtLiteralToken (TokenKind_FmtLiteral, *ts == '%'); m_parenthesesLevelStack.append (1); fcall main; };
'"' | nl  { createFmtLiteralToken (TokenKind_Literal); fret; };
any       ;

*|;

fmt_spec := |*

';' [^")\n]* { createFmtSpecifierToken (); fret; };
any          { ASSERT (false); fret; };

*|;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# global declarations & pragmas

'namespace'      { createToken (TokenKind_Namespace); };
'extension'      { createToken (TokenKind_Extension); };
'using'          { createToken (TokenKind_Using); };
'friend'         { createToken (TokenKind_Friend); };
'public'         { createToken (TokenKind_Public); };
'protected'      { createToken (TokenKind_Protected); };
'alignment'      { createToken (TokenKind_Alignment); };
'setas'          { createToken (TokenKind_SetAs); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# storage specifiers

'typedef'        { createToken (TokenKind_Typedef); };
'alias'          { createToken (TokenKind_Alias); };
'static'         { createToken (TokenKind_Static); };
'thread'         { createToken (TokenKind_Thread); };
'stack'          { createToken (TokenKind_Stack); };
'heap'           { createToken (TokenKind_Heap); };
'abstract'       { createToken (TokenKind_Abstract); };
'virtual'        { createToken (TokenKind_Virtual); };
'override'       { createToken (TokenKind_Override); };
'mutable'        { createToken (TokenKind_Mutable); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# type modifiers

'unsigned'       { createToken (TokenKind_Unsigned); };
'bigendian'      { createToken (TokenKind_BigEndian); };
'const'          { createToken (TokenKind_Const); };
'readonly'       { createToken (TokenKind_ReadOnly); };
'volatile'       { createToken (TokenKind_Volatile); };
'weak'           { createToken (TokenKind_Weak); };
'thin'           { createToken (TokenKind_Thin); };
'safe'           { createToken (TokenKind_Safe); };
'throws'         { createToken (TokenKind_Throws); };
'cdecl'          { createToken (TokenKind_Cdecl); };
'stdcall'        { createToken (TokenKind_Stdcall); };
'thiscall'       { createToken (TokenKind_Thiscall); };
'array'          { createToken (TokenKind_Array); };
'function'       { createToken (TokenKind_Function); };
'property'       { createToken (TokenKind_Property); };
'bindable'       { createToken (TokenKind_Bindable); };
'autoget'        { createToken (TokenKind_AutoGet); };
'indexed'        { createToken (TokenKind_Indexed); };
'multicast'      { createToken (TokenKind_Multicast); };
'event'          { createToken (TokenKind_Event); };
'reactor'        { createToken (TokenKind_Reactor); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# type specifiers

'auto'           { createToken (TokenKind_Auto); };
'void'           { createToken (TokenKind_Void); };
'object'         { createToken (TokenKind_Object); };
'variant'        { createToken (TokenKind_Variant); };
'bool'           { createToken (TokenKind_Bool); };
'int8'           { createToken (TokenKind_Int8); };
'int16'          { createToken (TokenKind_Int16); };
'int32'          { createToken (TokenKind_Int32); };
'int64'          { createToken (TokenKind_Int64); };
'float'          { createToken (TokenKind_Float); };
'double'         { createToken (TokenKind_Double); };
'int'            { createToken (TokenKind_Int); };
'intptr'         { createToken (TokenKind_IntPtr); };
'char'           { createToken (TokenKind_Char); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# named type specifiers

'enum'           { createToken (TokenKind_Enum); };
'struct'         { createToken (TokenKind_Struct); };
'union'          { createToken (TokenKind_Union); };
'class'          { createToken (TokenKind_Class); };
'opaque'         { createToken (TokenKind_Opaque); };
'exposed'        { createToken (TokenKind_Exposed); };
'bitflag'        { createToken (TokenKind_BitFlag); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# special member methods

'get'            { createToken (TokenKind_Get); };
'set'            { createToken (TokenKind_Set); };
'preconstruct'   { createToken (TokenKind_PreConstruct); };
'construct'      { createToken (TokenKind_Construct); };
'destruct'       { createToken (TokenKind_Destruct); };
'operator'       { createToken (TokenKind_Operator); };
'postfix'        { createToken (TokenKind_Postfix); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# operators

'new'            { createToken (TokenKind_New); };
'delete'         { createToken (TokenKind_Delete); };
'sizeof'         { createToken (TokenKind_SizeOf); };
'countof'        { createToken (TokenKind_CountOf); };
'offsetof'       { createToken (TokenKind_OffsetOf); };
'typeof'         { createToken (TokenKind_TypeOf); };
'bindingof'      { createToken (TokenKind_BindingOf); };
'dynamic'        { createToken (TokenKind_Dynamic); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# statements

'if'             { createToken (TokenKind_If); };
'else'           { createToken (TokenKind_Else); };
'for'            { createToken (TokenKind_For); };
'while'          { createToken (TokenKind_While); };
'do'             { createToken (TokenKind_Do); };
'break'          { createKeywordTokenEx (TokenKind_Break, 1); };
'break' [1-9]    { createKeywordTokenEx (TokenKind_Break, te [-1] - '0'); };
'continue'       { createKeywordTokenEx (TokenKind_Continue, 1); };
'continue' [1-9] { createKeywordTokenEx (TokenKind_Continue, te [-1] - '0'); };
'return'         { createToken (TokenKind_Return); };
'switch'         { createToken (TokenKind_Switch); };
'case'           { createToken (TokenKind_Case); };
'default'        { createToken (TokenKind_Default); };
'once'           { createToken (TokenKind_Once); };
'onevent'        { createToken (TokenKind_OnEvent); };
'try'            { createToken (TokenKind_Try); };
'catch'          { createToken (TokenKind_Catch); };
'finally'        { createToken (TokenKind_Finally); };
'assert'         { createToken (TokenKind_Assert); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# pre-defined values

'basetype'       { createKeywordTokenEx (TokenKind_BaseType, 1); };
'basetype' [1-9] { createKeywordTokenEx (TokenKind_BaseType, te [-1] - '0'); };
'this'           { createToken (TokenKind_This); };
'true'           { createToken (TokenKind_True); };
'false'          { createToken (TokenKind_False); };
'null'           { createToken (TokenKind_Null); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# symbol tokens

'++'             { createToken (TokenKind_Inc); };
'--'             { createToken (TokenKind_Dec); };
'->'             { createToken (TokenKind_Ptr); };
'=>'             { createToken (TokenKind_Imply); };
'<<'             { createToken (TokenKind_Shl); };
'>>'             { createToken (TokenKind_Shr); };
'&&'             { createToken (TokenKind_LogAnd); };
'||'             { createToken (TokenKind_LogOr); };
'=='             { createToken (TokenKind_Eq); };
'!='             { createToken (TokenKind_Ne); };
'<='             { createToken (TokenKind_Le); };
'>='             { createToken (TokenKind_Ge); };
':='             { createToken (TokenKind_RefAssign); };
'+='             { createToken (TokenKind_AddAssign); };
'-='             { createToken (TokenKind_SubAssign); };
'*='             { createToken (TokenKind_MulAssign); };
'/='             { createToken (TokenKind_DivAssign); };
'%='             { createToken (TokenKind_ModAssign); };
'<<='            { createToken (TokenKind_ShlAssign); };
'>>='            { createToken (TokenKind_ShrAssign); };
'&='             { createToken (TokenKind_AndAssign); };
'^='             { createToken (TokenKind_XorAssign); };
'|='             { createToken (TokenKind_OrAssign); };
'@='             { createToken (TokenKind_AtAssign); };
'...'            { createToken (TokenKind_Ellipsis); };

'$"'             { preCreateFmtLiteralToken (); fcall lit_fmt; };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# common tokens

id               { createStringToken (TokenKind_Identifier); };
lit_sq           { createCharToken (TokenKind_Integer); };
lit_dq           { createStringToken (TokenKind_Literal, 1, 1, true); };
dec+             { createIntegerToken (10); };
'0' [xx] hex+    { createIntegerToken (16, 2); };
'0' [xx] lit_dq  { createHexLiteralToken (); };
dec+ ('.' dec+) | ([ee] [+\-]? dec+)
				 { createFpToken (); };

'//' [^\n]*      ;
'/*' (any | nl)* :>> '*/'
				 ;

'('              { onLeftParentheses (); };
')'              { if (!onRightParentheses ()) fret; };
';'              { if (!onSemicolon ()) fcall fmt_spec; };

ws | nl          ;
print            { createToken (ts [0]); };
any              { createErrorToken (ts [0]); };

*|;

}%%

//.............................................................................

void
Lexer::init ()
{
	%% write init;
}

void
Lexer::exec ()
{
	%% write exec;
}

//.............................................................................

} // namespace jnc {
