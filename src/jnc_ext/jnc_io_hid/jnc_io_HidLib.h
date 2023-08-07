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

// {0F8C98F1-A8BB-49D1-BDBB-0DA7B1D9E549}
AXL_SL_DEFINE_GUID(
	g_hidLibGuid,
	0xf8c98f1, 0xa8bb, 0x49d1, 0xbd, 0xbb, 0xd, 0xa7, 0xb1, 0xd9, 0xe5, 0x49
);

enum HidLibTypeCacheSlot {
	HidLibTypeCacheSlot_HidDevice,
	HidLibTypeCacheSlot_HidDeviceDesc,
};

//..............................................................................

} // namespace io
} // namespace jnc

