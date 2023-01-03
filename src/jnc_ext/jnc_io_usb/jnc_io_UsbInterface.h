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

#include "jnc_io_UsbEndpoint.h"

namespace jnc {
namespace io {

class UsbDevice;
class UsbEndpoint;

JNC_DECLARE_OPAQUE_CLASS_TYPE(UsbInterface)

//..............................................................................

class UsbInterface: public IfaceHdr {
	friend class GetParentLink;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(UsbInterface)

public:
	class GetParentLink {
	public:
		sl::ListLink* operator()(UsbInterface* self) {
			return &self->m_parentLink;
		}
	};

public:
	UsbDevice* m_parentDevice;
	DataPtr m_interfaceDescriptorPtr;
	bool m_isClaimed;

protected:
	sys::Lock m_lock;
	sl::ListLink m_parentLink;
	sl::List<UsbEndpoint, UsbEndpoint::GetParentLink> m_endpointList;

public:
	UsbInterface();

	~UsbInterface() {
		release();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	void
	removeEndpoint(UsbEndpoint* endpoint);

	void
	JNC_CDECL
	release();

	UsbEndpoint*
	JNC_CDECL
	openEndpoint(
		uint8_t endpointId,
		bool isSuspended
	);
};

//..............................................................................

} // namespace io
} // namespace jnc
