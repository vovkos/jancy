#include "pch.h"
#include "jnc_ct_DoxyLexer.h"
#include "jnc_ct_DoxyLexer.rl.cpp"

namespace jnc {
namespace ct {

//.............................................................................

DoxyToken*
DoxyLexer::createTextToken ()
{
	size_t length = te - ts;
	
	sl::StringRef leftTrimmedString = sl::StringRef (ts, length).getLeftTrimmedString ();


	Token* token = createToken (DoxyTokenKind_Text);
	token->m_data.m_string.copy (ts, te - ts);
	return token;
}

//.............................................................................

} // namespace ct
} // namespace jnc
