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
exp    = [eE][+\-]?dec+;
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
	'pragma'          |
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
	'dynamic' ws+ 'field' |

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
	'bool'            |
	'int'             |
	'intptr'          |
	'char'            |
	'short'           |
	'long'            |
	'float'           |
	'double'          |
	'enum'            |
	'bitflag' ws+ 'enum' |
	'struct'          |
	'union'           |
	'class'           |
	'opaque' ws+ 'class' |

	'get'             |
	'set'             |
	'construct'       |
	'destruct'        |
	'operator'        |
	'postfix' ws+ 'operator' |

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
	'await'           |
	'dynamic' ws+ 'layout' |
	'dynamic' ws+ 'group' |

	'basetype' [1-9]? |
	'this'            |
	'true'            |
	'false'           |
	'null'            |

	'new'             |
	'delete'          |
	'sizeof'          |
	'countof'         |
	'typeof'          |
	'offsetof'        |
	'bindingof'       |
	'dynamic' ws+ 'sizeof' |
	'dynamic' ws+ 'countofof' |
	'dynamic' ws+ 'typeofof'
)                 { highlightLastToken(EditTheme::Keyword); };

'dynamic' ws+ '('
	{
		char* prev = te;
		te = ts + lengthof("dynamic");
		highlightLastToken(EditTheme::Keyword);
		te = prev;
	};

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

id                ;

(
	lit_dq            |
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
	(dec+ '.' dec* | '.' dec+)exp? | dec+ exp |
	[$fF] lit_dq
)                 { highlightLastToken(EditTheme::Constant); };

(('0' [xXoObBnNdD]) | [$fF])? '"""'
                  { highlightLastToken(EditTheme::Constant); fgoto lit_ml; };

'$' dec+          { highlightLastToken(EditTheme::Keyword); };

'//' any*         { highlightLastToken(EditTheme::Comment); setCurrentBlockState(BlockState_CommentSl); };
'/*'              { highlightLastToken(EditTheme::Comment); fgoto comment_ml; };

ws | nl           ;
any               ;

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# comment machine
#

comment_ml := |*

any* :>> '*/'?    { highlightLastToken(EditTheme::Comment); if (isTokenSuffix("*/", 2)) fgoto main; };

*|;

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# multi-line literal machine
#

lit_ml := |*

any* :>> '"""'?   { highlightLastToken(EditTheme::Constant); if (isTokenSuffix("\"\"\"", 3)) fgoto main; };

*|;

}%%

namespace jnc {

//..............................................................................

void
JancyHighlighter::init() {
	%% write init;
}

void
JancyHighlighter::exec() {
	int prevBlockState = previousBlockState();
	switch (prevBlockState) {
	case BlockState_CommentMl:
		cs = jancy_lexer_en_comment_ml;
		break;

	case BlockState_LitMl:
		cs = jancy_lexer_en_lit_ml;
		break;
	}

	setCurrentBlockState(BlockState_Normal);

	%% write exec;

	switch (cs) {
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
