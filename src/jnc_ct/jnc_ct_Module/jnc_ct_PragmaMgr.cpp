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

PragmaSettings::PragmaSettings()
{
	m_fieldAlignment = PragmaDefault_Alignment;
	m_pointerModifiers = PragmaDefault_PointerModifiers;
	m_enumFlags = PragmaDefault_EnumFlags;
}

//..............................................................................

} // namespace ct
} // namespace jnc
