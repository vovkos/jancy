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

#include "pch.h"
#include "jnc_ct_PragmaMgr.h"

namespace jnc {
namespace ct {

//..............................................................................

void
PragmaConfig::reset() {
	m_fieldAlignment = PragmaDefault_Alignment;
	m_pointerModifiers = 0;
	m_enumFlags = 0;
	m_regexFlags = 0;
	m_mask = 0;
}

//..............................................................................

} // namespace ct
} // namespace jnc
