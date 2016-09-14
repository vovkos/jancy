#include "pch.h"
#include "jnc_ct_DoxyLexer.h"
#include "jnc_ct_DoxyLexer.rl.cpp"

namespace jnc {
namespace ct {

//.............................................................................

DoxyToken*
DoxyLexer::createTextToken ()
{
	Token* token = createToken (DoxyTokenKind_Text);
	token->m_data.m_string.copy (ts, te - ts);
	return token;
}

DoxyToken*
DoxyLexer::createNewLineToken ()
{
	ASSERT (*ts == '\n');

	Token* token = createToken ('\n');
	token->m_data.m_string.copy (ts + 1, te - ts - 1);
	return token;
}

//.............................................................................

} // namespace ct
} // namespace jnc
