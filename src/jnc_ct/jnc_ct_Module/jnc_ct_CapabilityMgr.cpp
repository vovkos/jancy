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
#include "jnc_ct_CapabilityMgr.h"

namespace jnc {
namespace ct {

//..............................................................................

void
CapabilityMgr::initializeCapabilities(const sl::StringRef& initializer)
{
	sl::StringRef delimiters(" \t\r\n");

	m_capabilitySet.clear();
	m_isEverythingEnabled = false;

	for (size_t i = 0;;)
	{
		initializer.findNotOneOf(delimiters, i);
		if (i == -1)
			break;

		size_t end = initializer.findOneOf(delimiters, i);
		if (end == -1)
			break;

		sl::StringRef capability = initializer.getSubString(i, end - i);
		if (capability == "*")
		{
			m_isEverythingEnabled = true;
			break;
		}

		m_capabilitySet[capability] = true;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
