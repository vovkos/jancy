#include "pch.h"
#include "jnc_ct_DoxyLexer.h"
#include "jnc_ct_DoxyLexer.rl.cpp"

namespace jnc {
namespace ct {

//.............................................................................

DoxyToken*
DoxyLexer::createKeywordToken (DoxyTokenKind tokenKind)
{
	// create text token if any

	return createToken (tokenKind);
}

//.............................................................................

} // namespace ct
} // namespace jnc
