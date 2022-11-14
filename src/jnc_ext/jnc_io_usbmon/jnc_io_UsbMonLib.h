#pragma once

namespace jnc {
namespace io {

//..............................................................................

AXL_SELECT_ANY bool g_devMonCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requireDevMonCapability() {
	return g_devMonCapability || jnc::failWithCapabilityError("org.jancy.io.devmon");
}

//..............................................................................

// {CFEB2181-AAD4-4A85-9C5E-87B38A075FE5}
AXL_SL_DEFINE_GUID(
	g_usbMonLibGuid,
	0xcfeb2181, 0xaad4, 0x4a85, 0x9c, 0x5e, 0x87, 0xb3, 0x8a, 0x7, 0x5f, 0xe5
);

enum UsbMonLibTypeCacheSlot {
	UsbMonLibTypeCacheSlot_UsbMonitor,
	UsbMonLibTypeCacheSlot_UsbMonDeviceDesc,
};

//..............................................................................

} // namespace io
} // namespace jnc

