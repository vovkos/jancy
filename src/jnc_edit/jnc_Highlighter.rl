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

%%{

machine jancy_lexer;
write data;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# standard definitions
#

dec    = [0-9];
hex    = [0-9a-fA-F];
oct    = [0-7];
bin    = [01];
id     = [_a-zA-Z] [_a-zA-Z0-9]*;
ws     = [ \t\r]+;
nl     = '\n';
esc    = '\\' [^\n];

lit_dq     = '"' ([^"\n\\] | esc)* ["\\]?;
lit_sq     = "'" ([^'\n\\] | esc)* ['\\]?;
raw_lit_dq = '"' [^"\n]* '"'?;
raw_lit_sq = "'" [^'\n]* "'"?;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

(
'import'          |
'namespace'       |
'extension'       |
'dynamiclib'      |
'using'           |
'friend'          |
'public'          |
'protected'       |
'alignment'       |
'setas'           |

'typedef'         |
'alias'           |
'static'          |
'threadlocal'     |
'stack'           |
'heap'            |
'abstract'        |
'virtual'         |
'override'        |
'mutable'         |
'disposable'      |

'signed'          |
'unsigned'        |
'bigendian'       |
'const'           |
'cmut'            |
'readonly'        |
'volatile'        |
'safe'            |
'unsafe'          |
'weak'            |
'thin'            |
'errorcode'       |
'cdecl'           |
'stdcall'         |
'thiscall'        |
'jnccall'         |
'array'           |
'function'        |
'property'        |
'bindable'        |
'autoget'         |
'indexed'         |
'multicast'       |
'event'           |
'reactor'         |
'async'           |

'anydata'         |
'void'            |
'variant'         |
'bool'            |
'int'             |
'intptr'          |
'char'            |
'short'           |
'long'            |
'float'           |
'double'          |
'enum'            |
'struct'          |
'union'           |
'class'           |
'opaque'          |
'exposed'         |
'bitflag'         |

'get'             |
'set'             |
'construct'       |
'destruct'        |
'operator'        |
'postfix'         |

'if'              |
'else'            |
'for'             |
'while'           |
'do'              |
'break'           |
'break' [1-9]?    |
'continue' [1-9]? |
'return'          |
'switch'          |
'reswitch'        |
'case'            |
'default'         |
'once'            |
'onevent'         |
'try'             |
'throw'           |
'catch'           |
'finally'         |
'nestedscope'     |
'assert'          |
'await'           |

'basetype' [1-9]? |
'this'            |
'true'            |
'false'           |
'null'            |

'new'             |
'delete'          |
'sizeof'          |
'countof'         |
'offsetof'        |
'typeof'          |
'bindingof'       |
'dynamic'
)                 { highlightLastToken(Color_Keyword); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

id                ;

lit_dq            { highlightLastToken(Color_Constant, true); };

(
lit_sq            |
[rR] raw_lit_sq   |
[rR] raw_lit_dq   |
dec+              |
'0' oct+          |
'0' [xX] hex+     |
'0' [oO] oct+     |
'0' [bB] bin+     |
'0' [nNdD] dec+   |
'0' [xXoObBnNdD] raw_lit_dq |
dec+ (('.' dec*) | ([eE] [+\-]? dec+)) |
[$fF] lit_dq
)                 { highlightLastToken(Color_Constant); };

('0' [xXoObBnNdD])? '"""'
                  { highlightLastToken(Color_Constant); fgoto lit_ml; };

'//' any*         { highlightLastToken(Color_Comment); setCurrentBlockState(BlockState_CommentSl); };
'/*'              { highlightLastToken(Color_Comment); fgoto comment_ml; };

ws | nl           ;
any               ;

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# comment machine
#

comment_ml := |*

any* :>> '*/'?    { highlightLastToken(Color_Comment); if (isTokenSuffix("*/", 2)) fgoto main; };

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# multi-line literal machine
#

lit_ml := |*

any* :>> '"""'?   { highlightLastToken(Color_Constant); if (isTokenSuffix("\"\"\"", 3)) fgoto main; };

*|;

}%%

namespace jnc {

//..............................................................................

void
JancyHighlighter::init()
{
	%% write init;
}

void
JancyHighlighter::exec()
{
	int prevBlockState = previousBlockState();
	switch (prevBlockState)
	{
	case BlockState_CommentMl:
		cs = jancy_lexer_en_comment_ml;
		break;

	case BlockState_LitMl:
		cs = jancy_lexer_en_lit_ml;
		break;
	}

	setCurrentBlockState(BlockState_Normal);

	%% write exec;

	switch (cs)
	{
	case jancy_lexer_en_comment_ml:
		setCurrentBlockState(BlockState_CommentMl);
		break;

	case jancy_lexer_en_lit_ml:
		setCurrentBlockState(BlockState_LitMl);
		break;
	}
}

//..............................................................................

} // namespace jnc
