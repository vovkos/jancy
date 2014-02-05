#include "pch.h"
#include "llvmirhighlighter.h"
#include "moc_llvmirhighlighter.cpp"

//.............................................................................

%%{

machine llvmir_lexer; 
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
)				{ colorize(ts, te, Qt::blue); };

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

id								{  };
(lit_sq | lit_dq)				{ colorize(ts, te, Qt::darkRed); };
dec+							{ colorize(ts, te, Qt::darkRed); };
'0' [Xx] hex+					{ colorize(ts, te, Qt::darkRed); };

';' [^\n]*						{ colorize(ts, te, Qt::darkGreen); };

ws | nl							;

any								{  };

*|;

}%%

//.............................................................................

void LlvmIrHighlighter::ragelInit()
{
	%% write init;
}

void LlvmIrHighlighter::ragelExec()
{
	%% write exec;
}

int LlvmIrHighlighter::getRagelState(int blockState)
{
	switch (blockState)
	{
		case 1:
			return llvmir_lexer_en_main;
	}

	return llvmir_lexer_en_main;
}

//.............................................................................
