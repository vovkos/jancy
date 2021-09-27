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
namespace io {

//..............................................................................

union UsbVidPid {
	uint32_t m_vidPid;

	struct {
		uint16_t m_vid;
		uint16_t m_pid;
	};

	UsbVidPid() {
		m_vidPid = 0;
	}

	UsbVidPid(uint32_t vidPid) {
		m_vidPid = vidPid;
	}

	UsbVidPid(
		uint16_t vid,
		uint16_t pid
	) {
		m_vid = vid;
		m_pid = pid;
	}

	size_t
	hash() const {
		return m_vidPid;
	}

	bool
	isEqual(const UsbVidPid& src) const {
		return m_vidPid == src.m_vidPid;
	}
};

//..............................................................................

typedef sl::HashTable<
	UsbVidPid,
	bool,
	sl::HashDuckType<UsbVidPid>,
	sl::EqDuckType<UsbVidPid>
> UsbVidPidSet;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
UsbVidPidSet*
getEnabledUsbVidPidSet() {
	return sl::getSingleton<UsbVidPidSet>();
}

inline
void
enableUsbDeviceAccess(
	uint16_t vid,
	uint16_t pid
) {
	getEnabledUsbVidPidSet()->add(UsbVidPid(vid, pid), true);
}

inline
void
enableUsbDeviceAccess(uint32_t vidPid) {
	getEnabledUsbVidPidSet()->add(vidPid, true);
}

//..............................................................................

AXL_SELECT_ANY bool g_canAccessAllUsbDevices = true;

inline
bool
checkUsbDeviceAccess(
	uint16_t vid,
	uint16_t pid
) {
	return
		g_canAccessAllUsbDevices ||
		getEnabledUsbVidPidSet()->find(UsbVidPid(vid, pid)) ||
		err::fail(sl::formatString("access to USB VID%04X PID%04X denied", vid, pid));
}

//..............................................................................

} // namespace io
} // namespace jnc
