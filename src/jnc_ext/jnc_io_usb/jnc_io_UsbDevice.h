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
class UsbAsyncControlEndpoint;

JNC_DECLARE_OPAQUE_CLASS_TYPE (UsbDevice)

//..............................................................................

class UsbDevice: public IfaceHdr
{
	friend class UsbInterface;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (UsbDevice)

protected:
	bool m_isOpen;
	bool m_isAutoDetachKernelDriverEnabled;

protected:
	axl::io::UsbDevice m_device;
	UsbAsyncControlEndpoint* m_asyncControlEndpoint;

public:
	UsbDevice ();

	~UsbDevice ()
	{
		close ();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	axl::io::UsbDevice*
	getDevice ()
	{
		return &m_device;
	}

	void
	setDevice (libusb_device* srcDevice)
	{
		m_device.setDevice (srcDevice);
	}

	void
	takeOver (axl::io::UsbDevice* srcDevice)
	{
		m_isOpen = srcDevice->isOpen ();
		sl::takeOver (&m_device, srcDevice);
	}

	void
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	open ();

	static
	DataPtr
	JNC_CDECL
	getDeviceDesc (UsbDevice* self);

	static
	DataPtr
	JNC_CDECL
	getActiveConfigurationDesc (UsbDevice* self);

	uint8_t
	JNC_CDECL
	getConfigurationId ()
	{
		return m_isOpen ? m_device.getConfiguration () : 0;
	}

	bool
	JNC_CDECL
	setConfigurationId (uint8_t configurationId)
	{
		return m_isOpen ? m_device.setConfiguration (configurationId) : false;
	}

	uint8_t
	JNC_CDECL
	getBus ()
	{
		return m_device.getBusNumber ();
	}

	uint8_t
	JNC_CDECL
	getAddress ()
	{
		return m_device.getDeviceAddress ();
	}

	uint8_t
	JNC_CDECL
	getSpeed ()
	{
		return m_device.getDeviceSpeed ();
	}

	static
	DataPtr
	JNC_CDECL
	getStringDesc (
		UsbDevice* self,
		uint8_t stringId
		);

	bool
	JNC_CDECL
	setAutoDetachKernelDriverEnabled (bool isEnabled)
	{
		return m_device.setAutoDetachKernelDriver (isEnabled);
	}

	bool
	JNC_CDECL
	isKernelDriverActive (uint_t interfaceId)
	{
		return m_device.isKernelDriverActive (interfaceId);
	}

	bool
	JNC_CDECL
	attachKernelDriver (uint_t interfaceId)
	{
		return m_device.attachKernelDriver (interfaceId);
	}

	bool
	JNC_CDECL
	detachKernelDriver (uint_t interfaceId)
	{
		return m_device.detachKernelDriver (interfaceId);
	}

	UsbInterface*
	JNC_CDECL
	claimInterface (
		uint8_t interfaceId,
		uint8_t altSettingId
		);

	size_t
	JNC_CDECL
	controlTransfer_0 (
		uint_t requestType,
		uint_t requestCode,
		uint_t value,
		uint_t index,
		DataPtr ptr,
		size_t size,
		uint_t timeout
		);

	bool
	JNC_CDECL
	controlTransfer_1 (
		uint_t requestType,
		uint_t requestCode,
		uint_t value,
		uint_t index,
		DataPtr ptr,
		size_t size,
		uint_t timeout,
		FunctionPtr onCompletedPtr
		);

	void
	JNC_CDECL
	cancelControlTransfers ();
};

//..............................................................................

DataPtr
createUsbDeviceArray (DataPtr countPtr);

UsbDevice*
openUsbDevice (
	uint_t vendorId,
	uint_t productId
	);

//..............................................................................

} // namespace io
} // namespace jnc
