#pragma once

namespace jnc {
namespace io {

//..............................................................................

AXL_SELECT_ANY bool g_devMonCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requireDevMonCapability()
{
	return g_devMonCapability || jnc::failWithCapabilityError("org.jancy.io.devmon");
}

//..............................................................................

// {A9A83151-4834-472D-AF0B-43F9BE43D465}
AXL_SL_DEFINE_GUID(
	g_devMonLibGuid,
	0xa9a83151, 0x4834, 0x472d, 0xaf, 0xb, 0x43, 0xf9, 0xbe, 0x43, 0xd4, 0x65
	);

enum DevMonLibTypeCacheSlot
{
	DevMonLibTypeCacheSlot_DeviceMonitor,
};

//..............................................................................

} // namespace io
} // namespace jnc

