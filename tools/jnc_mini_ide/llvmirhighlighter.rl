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

machine llvmir_lexer;
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
lit_dq = '"' ([^"\n\\] | esc)* (["\\] | nl);
lit_sq = "'" ([^'\n\\] | esc)* (['\\] | nl);

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

(
'ret' |
'br' |
'switch' |
'indirectbr' |
'invoke' |
'resume' |
'unreachable' |
'add' |
'fadd' |
'sub' |
'fsub' |
'mul' |
'fmul' |
'udiv' |
'sdiv' |
'fdiv' |
'urem' |
'srem' |
'frem' |
'shl' |
'lshr' |
'ashr' |
'and' |
'or' |
'xor' |
'extractelement' |
'insertelement' |
'shufflevector' |
'extractvalue' |
'insertvalue' |
'alloca' |
'load' |
'store' |
'fence' |
'cmpxchq' |
'atomicrmw' |
'getelementptr' |
'trunc' |
'zext' |
'sext' |
'fptrunc' |
'fpext' |
'fptoui' |
'fptosi' |
'uitofp' |
'sitofp' |
'ptrtoint' |
'inttoptr' |
'bitcast' |
'icmp' |
'fcmp' |
'phi' |
'select' |
'call' |
'va_arg' |
'landingpad'
)                   { highlightLastToken(Color_Keyword); };

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

id                  { };
(lit_sq | lit_dq)   { highlightLastToken(Color_Constant); };
dec+              	{ highlightLastToken(Color_Constant); };
'0' [Xx] hex+       { highlightLastToken(Color_Constant); };

';' [^\n]*          { highlightLastToken(Color_Comment); };

ws | nl             ;

any                 { };

*|;

}%%

//..............................................................................

void
LlvmIrHighlighter::init() {
	%% write init;
}

void
LlvmIrHighlighter::exec() {
	%% write exec;
}

//..............................................................................
