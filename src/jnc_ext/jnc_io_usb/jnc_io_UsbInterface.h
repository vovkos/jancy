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

class UsbDevice;
class UsbEndpoint;

JNC_DECLARE_OPAQUE_CLASS_TYPE(UsbInterface)

//..............................................................................

class UsbInterface: public IfaceHdr
{
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(UsbInterface)

public:
	UsbDevice* m_parentDevice;
	DataPtr m_interfaceDescPtr;

	bool m_isClaimed;

public:
	UsbInterface();

	~UsbInterface()
	{
		release();
	}

	void
	JNC_CDECL
	release();

	UsbEndpoint*
	JNC_CDECL
	openEndpoint(uint8_t endpointId);
};

//..............................................................................

} // namespace io
} // namespace jnc
