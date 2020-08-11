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
lc_nl  = '\\' '\r'? nl;
esc    = '\\' [^\n];

lit_dq     = '"' ([^"\n\\] | esc)* (["\\] | nl);
lit_sq     = "'" ([^'\n\\] | esc)* (['\\] | nl);
raw_lit_dq = '"' [^"\n]* ('"' | nl);
raw_lit_sq = "'" [^'\n]* ("'" | nl);

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

(
lit_sq            |
lit_dq            |
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

'//' any*         { highlightLastToken(Color_Comment); };
'/*'              { highlightLastToken(Color_Comment); fgoto comment; };

ws | nl           ;
any               ;

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# comment machine
#

comment := |*

'*/'              { highlightLastToken(Color_Comment); fgoto main; };
any               { highlightLastToken(Color_Comment); };

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# multi-line literal machine
#

lit_ml := |*

'"""'             { highlightLastToken(Color_Constant); fgoto main; };
any               { highlightLastToken(Color_Constant); };

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
	case BlockState_Comment:
		cs = jancy_lexer_en_comment;
		break;

	case BlockState_LitMl:
		cs = jancy_lexer_en_lit_ml;
		break;
	}

	%% write exec;

	switch (cs)
	{
	case jancy_lexer_en_comment:
		setCurrentBlockState(BlockState_Comment);
		break;

	case jancy_lexer_en_lit_ml:
		setCurrentBlockState(BlockState_LitMl);
		break;

	default:
		setCurrentBlockState(BlockState_Normal);
	}
}

//..............................................................................

} // namespace jnc
