#include "jnc_Lexer.h"

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
	stack = PrePush ();
}

postpop
{
	PostPop ();
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
nl     = '\n' @{ NewLine (p + 1); };
esc    = '\\' [^\n];
lit_dq = '"' ([^"\n\\] | esc)* (["\\] | nl);
lit_sq = "'" ([^'\n\\] | esc)* (['\\] | nl);

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# formatting literal machines
#

lit_fmt := |*

esc       ;
'$' id    { CreateFmtSimpleIdentifierToken (); };
'$('      { CreateFmtLiteralToken (EToken_FmtLiteral); m_ParenthesesLevelStack.Append (1); fcall main; };
'"' | nl  { CreateFmtLiteralToken (EToken_Literal); fret; };
any       ;

*|;

fmt_spec := |*

',' [^")\n]* { CreateFmtSpecifierToken (); fret; };
any          { ASSERT (false); fret; };

*|;


# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# global declarations & pragmas

'namespace'      { CreateToken (EToken_Namespace); };
'using'          { CreateToken (EToken_Using); };
'extend'         { CreateToken (EToken_Extend); };
'pack'           { CreateToken (EToken_Pack); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# storage specifiers

'typedef'        { CreateToken (EToken_Typedef); };
'alias'          { CreateToken (EToken_Alias); };
'static'         { CreateToken (EToken_Static); };
'thread'         { CreateToken (EToken_Thread); };
'stack'          { CreateToken (EToken_Stack); };
'heap'           { CreateToken (EToken_Heap); };
'uheap'          { CreateToken (EToken_UHeap); };
'abstract'       { CreateToken (EToken_Abstract); };
'virtual'        { CreateToken (EToken_Virtual); };
'override'       { CreateToken (EToken_Override); };
'mutable'        { CreateToken (EToken_Mutable); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# access specifiers

'public'         { CreateToken (EToken_Public); };
'protected'      { CreateToken (EToken_Protected); };
'friend'         { CreateToken (EToken_Friend); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# type modifiers

'signed'         { CreateToken (EToken_Signed); };
'unsigned'       { CreateToken (EToken_Unsigned); };
'bigendian'      { CreateToken (EToken_BigEndian); };
'const'          { CreateToken (EToken_Const); };
'dconst'         { CreateToken (EToken_DConst); };
'volatile'       { CreateToken (EToken_Volatile); };
'weak'           { CreateToken (EToken_Weak); };
'thin'           { CreateToken (EToken_Thin); };
'safe'           { CreateToken (EToken_Safe); };
'throws'         { CreateToken (EToken_Throws); };
'cdecl'          { CreateToken (EToken_Cdecl); };
'stdcall'        { CreateToken (EToken_Stdcall); };
'thiscall'       { CreateToken (EToken_Thiscall); };
'array'          { CreateToken (EToken_Array); };
'function'       { CreateToken (EToken_Function); };
'property'       { CreateToken (EToken_Property); };
'bindable'       { CreateToken (EToken_Bindable); };
'autoget'        { CreateToken (EToken_AutoGet); };
'indexed'        { CreateToken (EToken_Indexed); };
'multicast'      { CreateToken (EToken_Multicast); };
'event'          { CreateToken (EToken_Event); };
'devent'         { CreateToken (EToken_DEvent); };
'reactor'        { CreateToken (EToken_Reactor); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# type specifiers

'auto'           { CreateToken (EToken_Auto); };
'void'           { CreateToken (EToken_Void); };
'object'         { CreateToken (EToken_Object); };
'variant'        { CreateToken (EToken_Variant); };
'bool'           { CreateToken (EToken_Bool); };
'int8'           { CreateToken (EToken_Int8); };
'int16'          { CreateToken (EToken_Int16); };
'int32'          { CreateToken (EToken_Int32); };
'int64'          { CreateToken (EToken_Int64); };
'float'          { CreateToken (EToken_Float); };
'double'         { CreateToken (EToken_Double); };
'int'            { CreateToken (EToken_Int); };
'intptr'         { CreateToken (EToken_IntPtr); };
'char'           { CreateToken (EToken_Char); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# named type specifiers

'enum'           { CreateToken (EToken_Enum); };
'fenum'          { CreateToken (EToken_FEnum); };
'cenum'          { CreateToken (EToken_CEnum); };
'struct'         { CreateToken (EToken_Struct); };
'union'          { CreateToken (EToken_Union); };
'class'          { CreateToken (EToken_Class); };
'opaque'         { CreateToken (EToken_Opaque); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# special member methods

'get'            { CreateToken (EToken_Get); };
'set'            { CreateToken (EToken_Set); };
'preconstruct'   { CreateToken (EToken_PreConstruct); };
'construct'      { CreateToken (EToken_Construct); };
'destruct'       { CreateToken (EToken_Destruct); };
'operator'       { CreateToken (EToken_Operator); };
'postfix'        { CreateToken (EToken_Postfix); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# operators

'new'            { CreateToken (EToken_New); };
'pnew'           { CreateToken (EToken_PNew); };
'delete'         { CreateToken (EToken_Delete); };
'sizeof'         { CreateToken (EToken_SizeOf); };
'countof'        { CreateToken (EToken_CountOf); };
'offsetof'       { CreateToken (EToken_OffsetOf); };
'typeof'         { CreateToken (EToken_TypeOf); };
'dtypeof'        { CreateToken (EToken_DTypeOf); };
'bindingof'      { CreateToken (EToken_BindingOf); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# statements

'if'             { CreateToken (EToken_If); };
'else'           { CreateToken (EToken_Else); };
'for'            { CreateToken (EToken_For); };
'while'          { CreateToken (EToken_While); };
'do'             { CreateToken (EToken_Do); };
'break'          { CreateKeywordTokenEx (EToken_Break, 1); };
'break' [1-9]    { CreateKeywordTokenEx (EToken_Break, te [-1] - '0'); };
'continue'       { CreateKeywordTokenEx (EToken_Continue, 1); };
'continue' [1-9] { CreateKeywordTokenEx (EToken_Continue, te [-1] - '0'); };
'return'         { CreateToken (EToken_Return); };
'switch'         { CreateToken (EToken_Switch); };
'case'           { CreateToken (EToken_Case); };
'default'        { CreateToken (EToken_Default); };
'once'           { CreateToken (EToken_Once); };
'onevent'        { CreateToken (EToken_OnEvent); };
'try'            { CreateToken (EToken_Try); };
'catch'          { CreateToken (EToken_Catch); };
'finally'        { CreateToken (EToken_Finally); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# pre-defined values

'basetype'       { CreateKeywordTokenEx (EToken_BaseType, 1); };
'basetype' [1-9] { CreateKeywordTokenEx (EToken_BaseType, te [-1] - '0'); };
'this'           { CreateToken (EToken_This); };
'retval'         { CreateToken (EToken_RetVal); };
'true'           { CreateToken (EToken_True); };
'false'          { CreateToken (EToken_False); };
'null'           { CreateToken (EToken_Null); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# symbol tokens

'++'             { CreateToken (EToken_Inc); };
'--'             { CreateToken (EToken_Dec); };
'->'             { CreateToken (EToken_Ptr); };
'=>'             { CreateToken (EToken_Imply); };
'<<'             { CreateToken (EToken_Shl); };
'>>'             { CreateToken (EToken_Shr); };
'&&'             { CreateToken (EToken_LogAnd); };
'||'             { CreateToken (EToken_LogOr); };
'=='             { CreateToken (EToken_Eq); };
'!='             { CreateToken (EToken_Ne); };
'<='             { CreateToken (EToken_Le); };
'>='             { CreateToken (EToken_Ge); };
':='             { CreateToken (EToken_RefAssign); };
'+='             { CreateToken (EToken_AddAssign); };
'-='             { CreateToken (EToken_SubAssign); };
'*='             { CreateToken (EToken_MulAssign); };
'/='             { CreateToken (EToken_DivAssign); };
'%='             { CreateToken (EToken_ModAssign); };
'<<='            { CreateToken (EToken_ShlAssign); };
'>>='            { CreateToken (EToken_ShrAssign); };
'&='             { CreateToken (EToken_AndAssign); };
'^='             { CreateToken (EToken_XorAssign); };
'|='             { CreateToken (EToken_OrAssign); };
'@='             { CreateToken (EToken_AtAssign); };
'...'            { CreateToken (EToken_Ellipsis); };

'$"'             { PreCreateFmtLiteralToken (); fcall lit_fmt; };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# common tokens

id               { CreateStringToken (EToken_Identifier); };
lit_sq           { CreateCharToken (EToken_Integer); };
lit_dq           { CreateStringToken (EToken_Literal, 1, 1, true); };
dec+             { CreateIntegerToken (10); };
'0' [Xx] hex+    { CreateIntegerToken (16, 2); };
'0' [Xx] lit_dq  { CreateHexLiteralToken (); };
dec+ ('.' dec+) | ([Ee] [+\-]? dec+)
				 { CreateFpToken (); };

'//' [^\n]*      ;
'/*' (any | nl)* :>> '*/'
				 ;

'('              { OnLeftParentheses (); };
')'              { if (!OnRightParentheses ()) fret; };
','              { if (!OnComma ()) fcall fmt_spec; };

ws | nl          ;
print            { CreateToken (ts [0]); };
any              { CreateErrorToken (ts [0]); };

*|;

}%%

//.............................................................................

void
CLexer::Init ()
{
	%% write init;
}

void
CLexer::Exec ()
{
	%% write exec;
}

//.............................................................................

} // namespace jnc {


