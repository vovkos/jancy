//.............................................................................

%%{

machine jancy_lexer;
write data;

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
nl     = '\n';
lc_nl  = '\\' '\r'? nl;
esc    = '\\' [^\n];
lit_dq = '"' ([^"\n\\] | esc)* (["\\] | nl);
lit_sq = "'" ([^'\n\\] | esc)* (['\\] | nl);

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

(
'import'          |
'namespace'       |
'extension'       |
'library'         |
'using'           |
'friend'          |
'public'          |
'protected'       |
'alignment'       |
'setas'           |

'typedef'         |
'alias'           |
'static'          |
'thread'          |
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
'automaton'       |

'auto'            |
'anydata'         |
'void'            |
'variant'         |
'bool'            |
'int8'            |
'int16'           |
'int32'           |
'int64'           |
'float'           |
'double'          |
'char'            |
'int'             |
'intptr'          |
'enum'            |
'struct'          |
'union'           |
'class'           |
'opaque'          |
'exposed'         |
'bitflag'         |

'get'             |
'set'             |
'preconstruct'    |
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

)                   { colorize (ts, te, Qt::blue); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

id                  ;
(lit_sq | lit_dq)   { colorize (ts, te, Qt::darkRed); };
dec+                { colorize (ts, te, Qt::darkRed); };
'0' [xX] hex+       { colorize (ts, te, Qt::darkRed); };
'0' [bB] bin+       { colorize (ts, te, Qt::darkRed); };
'0' [xX] lit_dq     { colorize (ts, te, Qt::darkRed); };
'0' [bB] lit_dq     { colorize (ts, te, Qt::darkRed); };
'$' lit_dq          { colorize (ts, te, Qt::darkRed); };

'%%'                { colorize (ts, te, Qt::darkRed); fgoto regexp; };
'<<<'               { colorize (ts, te, Qt::darkRed); fgoto lit_ml; };

'//' any*           { colorize (ts, te, Qt::darkGray); };
'/*'                { colorize (ts, te, Qt::darkGray); fgoto comment; };

ws | nl             ;
any                 ;

*|;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# comment machine
#

comment := |*

'*/'                { colorize (ts, te, Qt::darkGray); fgoto main; };
any                 { colorize (ts, te, Qt::darkGray); };

*|;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# regexp machine
#

regexp := |*

any* '\\'           { colorize (ts, te, Qt::darkRed); };
any*                { colorize (ts, te, Qt::darkRed); fgoto main; };

*|;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# multi-line literal machine
#

lit_ml := |*

'>>>'               { colorize (ts, te, Qt::darkRed); fgoto main; };
any                 { colorize (ts, te, Qt::darkRed); };

*|;

}%%

//.............................................................................

#define BLOCK_STATE_NONE	0
#define BLOCK_STATE_COMMENT 1
#define BLOCK_STATE_REGEXP  2
#define BLOCK_STATE_LIT_ML  3

void JancyHighlighter::ragelInit ()
{
	%% write init;
}

void JancyHighlighter::ragelExec ()
{
	%% write exec;
}

void JancyHighlighter::ragelExecPreEvent (int &ragelState)
{
	setCurrentBlockState (BLOCK_STATE_NONE);

	int prevBlockState = previousBlockState ();	
	switch (prevBlockState)
	{
	case BLOCK_STATE_COMMENT:
		ragelState = jancy_lexer_en_comment;
		break;

	case BLOCK_STATE_REGEXP:
		ragelState = jancy_lexer_en_regexp;
		break;

	case BLOCK_STATE_LIT_ML:
		ragelState = jancy_lexer_en_lit_ml;
		break;
	}
}

void JancyHighlighter::ragelExecPostEvent (int ragelState)
{
	switch (ragelState)
	{
	case jancy_lexer_en_comment:
		setCurrentBlockState (BLOCK_STATE_COMMENT);
		break;

	case jancy_lexer_en_regexp:
		setCurrentBlockState (BLOCK_STATE_REGEXP);
		break;

	case jancy_lexer_en_lit_ml:
		setCurrentBlockState (BLOCK_STATE_LIT_ML);
		break;
	}
}

//.............................................................................
