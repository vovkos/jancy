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

size_t
CapabilityMgr::readCapabilityParam(
	const char* param,
	void* value,
	size_t size
) {
	sl::StringHashTableIterator<sl::Array<char> > it = m_paramSet.find(param);
	if (!it)
		return 0;

	size_t copySize = it->m_value.getCount();
	if (!size)
		return copySize;

	if (copySize > size)
		copySize = size;

	memcpy(value, it->m_value, copySize);
	return copySize;
}

size_t
CapabilityMgr::writeCapabilityParam(
	const char* param,
	const void* value,
	size_t size
) {
	sl::StringHashTableIterator<sl::Array<char> > it = m_paramSet.visit(param);
	it->m_value.copy((char*)value, size);
	return size;
}

void
CapabilityMgr::initializeCapabilities(const sl::StringRef& initializer) {
	sl::StringRef delimiters(",;/ \t\r\n");

	m_capabilitySet.clear();
	m_isEverythingEnabled = false;

	for (size_t i = 0;;) {
		i = initializer.findNotOneOf(delimiters, i);
		if (i == -1)
			break;

		size_t end = initializer.findOneOf(delimiters, i);
		if (end == -1)
			end = initializer.getLength();

		sl::StringRef capability = initializer.getSubString(i, end - i);
		if (capability == "*") {
			m_isEverythingEnabled = true;
			break;
		}

		m_capabilitySet[capability] = true;
		i = end;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
