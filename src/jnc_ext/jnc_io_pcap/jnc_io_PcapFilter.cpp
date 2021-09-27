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
#include "jnc_io_PcapFilter.h"
#include "jnc_io_Pcap.h"
#include "jnc_io_PcapLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	PcapFilter,
	"io.PcapFilter",
	g_pcapLibGuid,
	PcapLibCacheSlot_PcapFilter,
	PcapFilter,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(PcapFilter)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<PcapFilter>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<PcapFilter>)

	JNC_MAP_FUNCTION("compile", &PcapFilter::compile_0)
	JNC_MAP_OVERLOAD(&PcapFilter::compile_1)
	JNC_MAP_FUNCTION("match",   &PcapFilter::match)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

bool
JNC_CDECL
PcapFilter::compile_0(
	Pcap* pcap,
	DataPtr filterPtr,
	bool isOptimized,
	uint_t netMask
) {
	if (!filterPtr.m_p) {
		m_filter.free();
		return true;
	}

	return m_filter.compile(pcap->getPcap(), (char*)filterPtr.m_p, isOptimized, netMask);
}

bool
JNC_CDECL
PcapFilter::compile_1(
	int linkType,
	size_t snapshotSize,
	DataPtr filterPtr,
	bool isOptimized,
	uint_t netMask
) {
	if (!filterPtr.m_p) {
		m_filter.free();
		return true;
	}

	return m_filter.compile(linkType, snapshotSize, (char*)filterPtr.m_p, isOptimized, netMask);
}

//..............................................................................

} // namespace io
} // namespace jnc
