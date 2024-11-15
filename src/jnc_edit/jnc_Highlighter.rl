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

lit_dq  = '"' ([^"\n\\] | esc)* ["\\]?;
lit_sq  = "'" ([^'\n\\] | esc)* ['\\]?;
lit_dq_raw = '"' [^"\n]* '"'?;
lit_sq_raw = "'" [^'\n]* "'"?;

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
	'dylib'           |
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
	'dyfield'         |

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
	'dylayout'        |

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
	'declof'          |
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
	lit_dq       |
	lit_sq       |
	[rR] lit_sq_raw |
	[rR] lit_dq_raw |
	[$fF] lit_dq |
	'0' [xXoObBnNdD] lit_dq_raw |

	dec+              |
	'0' oct+          |
	'0' [xX] hex+     |
	'0' [oO] oct+     |
	'0' [bB] bin+     |
	'0' [nNdD] dec+   |
	(dec+ '.' dec* | '.' dec+)exp? | dec+ exp
)                 { highlightLastToken(EditTheme::Constant); };

[$fFrR] '"""'
                  { highlightLastToken(EditTheme::Constant); fgoto lit_ml; };

(('0' [xXoObBnNdD]) | [rR])? '"""'
                  { highlightLastToken(EditTheme::Constant); fgoto lit_ml_raw; };

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
# multi-line literal machines
#

lit_ml := |*

(esc | any)* :>> '"""'?
                  { highlightLastToken(EditTheme::Constant); if (isTokenSuffix("\"\"\"", 3)) fgoto main; };

*|;

lit_ml_raw := |*

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

	case BlockState_LitMlRaw:
		cs = jancy_lexer_en_lit_ml_raw;
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

	case jancy_lexer_en_lit_ml_raw:
		setCurrentBlockState(BlockState_LitMlRaw);
		break;
	}
}

//..............................................................................

} // namespace jnc
