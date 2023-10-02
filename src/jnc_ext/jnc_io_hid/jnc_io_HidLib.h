#pragma once

namespace jnc {
namespace io {

//..............................................................................

AXL_SELECT_ANY bool g_hidCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requireHidCapability() {
	return g_hidCapability || jnc::failWithCapabilityError("org.jancy.io.usb");
}

//..............................................................................

JNC_DECLARE_LIB(HidLib)

// {0F8C98F1-A8BB-49D1-BDBB-0DA7B1D9E549}
JNC_DEFINE_GUID(
	g_hidLibGuid,
	0xf8c98f1, 0xa8bb, 0x49d1, 0xbd, 0xbb, 0xd, 0xa7, 0xb1, 0xd9, 0xe5, 0x49
);

enum HidLibCacheSlot {
	HidLibCacheSlot_HidDevice,
	HidLibCacheSlot_HidDeviceDesc,
	HidLibCacheSlot_HidMonDeviceDesc,
	HidLibCacheSlot_HidReportField,
	HidLibCacheSlot_HidReport,
	HidLibCacheSlot_HidStandaloneReport,
	HidLibCacheSlot_HidRdCollection,
	HidLibCacheSlot_HidRd,
	HidLibCacheSlot_HidUsagePage,
	HidLibCacheSlot_HidDb,
};

//..............................................................................

} // namespace io
} // namespace jnc
