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

class Pcap;

JNC_DECLARE_OPAQUE_CLASS_TYPE(PcapFilter)

//..............................................................................

class PcapFilter: public IfaceHdr
{
protected:
	axl::io::PcapFilter m_filter;

public:
	bool
	JNC_CDECL
	compile_0(
		Pcap* pcap,
		DataPtr filterPtr,
		bool isOptimized,
		uint_t netMask
		);

	bool
	JNC_CDECL
	compile_1(
		size_t snapshotSize,
		int linkType,
		DataPtr filterPtr,
		bool isOptimized,
		uint_t netMask
		)
	{
		return m_filter.compile(snapshotSize, linkType, (char*)filterPtr.m_p, isOptimized, netMask);
	}

	bool
	JNC_CDECL
	match(
		DataPtr dataPtr,
		size_t size
		)
	{
		return m_filter.match(dataPtr.m_p, size);
	}
};

//..............................................................................

} // namespace io
} // namespace jnc
