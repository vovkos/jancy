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

class UsbInterface;

JNC_DECLARE_TYPE (UsbEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (UsbEndpoint)

//..............................................................................

enum UsbEventCode
{
	UsbEventCode_ReadyRead = 0,
	UsbEventCode_Eof,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct UsbEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (UsbEventParams)

	UsbEventCode m_eventCode;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//..............................................................................

class UsbEndpoint: public IfaceHdr
{
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (UsbEndpoint)

public:
	UsbInterface* m_parentInterface;
	DataPtr m_endpointDescPtr;
	ClassBox <Multicast> m_onEndpointEvent;

protected:

public:
	UsbEndpoint ();

	~UsbEndpoint ()
	{
		close ();
	}

	void
	close ();

	DataPtr
	JNC_CDECL
	getEndpointDesc (UsbEndpoint* self);

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t size
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
