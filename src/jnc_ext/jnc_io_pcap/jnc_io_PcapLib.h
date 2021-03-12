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

AXL_SELECT_ANY bool g_pcapCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requirePcapCapability()
{
	return g_pcapCapability || jnc::failWithCapabilityError("org.jancy.io.pcap");
}

//..............................................................................

// {72C7158B-F297-4F88-83A7-96E7FB548B29}
JNC_DEFINE_GUID(
	g_pcapLibGuid,
	0x72c7158b, 0xf297, 0x4f88, 0x83, 0xa7, 0x96, 0xe7, 0xfb, 0x54, 0x8b, 0x29
	);

enum PcapLibCacheSlot
{
	PcapLibCacheSlot_Pcap,
	PcapLibCacheSlot_PcapFilter,
	PcapLibCacheSlot_PcapAddress,
	PcapLibCacheSlot_PcapDeviceDesc,
};

//..............................................................................

} // namespace io
} // namespace jnc
