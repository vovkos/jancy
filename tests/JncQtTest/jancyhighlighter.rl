#include "jancyhighlighter.h"

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
'pack'            |
'namespace'       |
'extend'          |
'using'           |
'friend'          |
'public'          |
'protected'       |
'setas'           |

'typedef'         |
'alias'           |
'static'          |
'thread'          |
'stack'           |
'heap'            |
'uheap'           |
'abstract'        |
'virtual'         |
'override'        |
'mutable'         |

'signed'          |
'unsigned'        |
'bigendian'       |
'const'           |
'dconst'          |
'volatile'        |
'safe'            |
'weak'            |
'thin'            |
'throws'          |
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
'devent'          |
'reactor'         |

'auto'            |
'void'            |
'object'          |
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
'fenum'           |
'cenum'           |
'struct'          |
'union'           |
'class'           |
'opaque'          |

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
'catch'           |
'finally'         |

'basetype' [1-9]? |
'this'            |
'retval'          |
'true'            |
'false'           |
'null'            |

'new'             |
'pnew'            |
'delete'          |
'sizeof'          |
'countof'         |
'offsetof'        |
'typeof'          |
'dtypeof'         |
'bindingof'

)                   { colorize(ts, te, Qt::blue); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

id                  ;
(lit_sq | lit_dq)   { colorize(ts, te, Qt::darkRed); };
dec+                { colorize(ts, te, Qt::darkRed); };
'0' [Xx] hex+       { colorize(ts, te, Qt::darkRed); };
'0' [Xx] lit_dq     { colorize(ts, te, Qt::darkRed); };
'$' lit_dq          { colorize(ts, te, Qt::darkRed); };

'//' [^\n]*         { colorize(ts, te, Qt::darkGray); };

'/*'                { colorize(ts, te, Qt::darkGray); fgoto comment; };

ws | nl             ;
any                 ;

*|;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# comment machine
#

comment := |*

'\n'                { colorize(ts, te, Qt::darkGray); };
any                 { colorize(ts, te, Qt::darkGray); };
'*/'                { colorize(ts, te, Qt::darkGray); fgoto main; };

*|;

}%%

//.............................................................................

#define BLOCK_STATE_NONE	0
#define BLOCK_STATE_COMMENT 1

void JancyHighlighter::ragelInit()
{
	%% write init;
}

void JancyHighlighter::ragelExec()
{
	%% write exec;
}

int JancyHighlighter::getRagelState(int blockState)
{
	switch (blockState)
	{
		case 1:
			return jancy_lexer_en_comment;
	}

	return jancy_lexer_en_main;
}

void JancyHighlighter::ragelExecPreEvent(int &ragelState)
{
	setCurrentBlockState(BLOCK_STATE_NONE);

	if (previousBlockState() == BLOCK_STATE_COMMENT)
		ragelState = jancy_lexer_en_comment;
}

void JancyHighlighter::ragelExecPostEvent(int ragelState)
{
	if (ragelState == jancy_lexer_en_comment)
		setCurrentBlockState(BLOCK_STATE_COMMENT);
}

//.............................................................................
