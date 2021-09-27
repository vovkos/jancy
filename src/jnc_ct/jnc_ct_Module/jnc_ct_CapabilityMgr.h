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

#pragma once

namespace jnc {
namespace ct {

//..............................................................................

class CapabilityMgr {
	friend class Module;

protected:
	sl::StringHashTable<bool> m_capabilitySet;
	sl::StringHashTable<sl::Array<char> > m_paramSet;
	bool m_isEverythingEnabled;

public:
	CapabilityMgr() {
		m_isEverythingEnabled = true; // everything enabled by default
	}

	bool
	isEverythingEnabled() {
		return m_isEverythingEnabled;
	}

	bool
	isCapabilityEnabled(const sl::StringRef& capability) {
		return m_isEverythingEnabled || m_capabilitySet.findValue(capability, false);
	}

	size_t
	readCapabilityParam(
		const char* param,
		void* value,
		size_t size
	);

	size_t
	writeCapabilityParam(
		const char* param,
		const void* value,
		size_t size
	);

	void
	enableCapability(
		const sl::StringRef& capability,
		bool isEnabled = true
	) {
		m_isEverythingEnabled = false;
		m_capabilitySet[capability] = isEnabled;
	}

	void
	initializeCapabilities(const sl::StringRef& initializer);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
CapabilityMgr*
getCapabilityMgr() {
	return sl::getSingleton<CapabilityMgr>();
}

//..............................................................................

} // namespace ct
} // namespace jnc
