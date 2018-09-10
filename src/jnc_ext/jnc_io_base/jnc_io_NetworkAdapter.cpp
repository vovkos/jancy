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
#include "jnc_io_NetworkAdapter.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	NetworkAdapterAddress,
	"io.NetworkAdapterAddress",
	g_ioLibGuid,
	IoLibCacheSlot_NetworkAdapterAddress
	)

JNC_DEFINE_TYPE (
	NetworkAdapterDesc,
	"io.NetworkAdapterDesc",
	g_ioLibGuid,
	IoLibCacheSlot_NetworkAdapterDesc
	)

//..............................................................................

DataPtr
createNetworkAdapterAddress (
	Runtime* runtime,
	const axl::io::NetworkAdapterAddress* srcAddress,
	NetworkAdapterAddress* prevAddress
	)
{
	DataPtr addressPtr = createData <NetworkAdapterAddress> (runtime);
	NetworkAdapterAddress* address = (NetworkAdapterAddress*) addressPtr.m_p;
	address->m_address = srcAddress->m_address;
	address->m_netMaskBitCount = srcAddress->m_netMaskBitCount;

	if (prevAddress)
		prevAddress->m_nextPtr = addressPtr;

	return addressPtr;
}

DataPtr
createNetworkAdapterDesc (
	Runtime* runtime,
	const axl::io::NetworkAdapterDesc* srcAdapter,
	NetworkAdapterDesc* prevAdapter
	)
{
	DataPtr adapterPtr = createData <NetworkAdapterDesc> (runtime);
	NetworkAdapterDesc* adapter = (NetworkAdapterDesc*) adapterPtr.m_p;
	adapter->m_type = srcAdapter->getType ();
	adapter->m_flags = srcAdapter->getFlags ();
	adapter->m_namePtr = strDup (srcAdapter->getName ());
	adapter->m_descriptionPtr = strDup (srcAdapter->getDescription ());
	memcpy (adapter->m_mac, srcAdapter->getMac (), 6);

	if (prevAdapter)
		prevAdapter->m_nextPtr = adapterPtr;

	sl::ConstList <axl::io::NetworkAdapterAddress> addressList = srcAdapter->getAddressList ();
	if (addressList.isEmpty ())
		return adapterPtr;

	sl::ConstIterator <axl::io::NetworkAdapterAddress> it = addressList.getHead ();

	DataPtr addressPtr = createNetworkAdapterAddress (runtime, *it, NULL);
	adapter->m_addressPtr = addressPtr;
	adapter->m_addressCount = addressList.getCount ();

	for (it++; it; it++)
		addressPtr = createNetworkAdapterAddress (runtime, *it, (NetworkAdapterAddress*) addressPtr.m_p);

	return adapterPtr;
}

DataPtr
createNetworkAdapterDescList (
	DataPtr adapterCountPtr,
	DataPtr addressCountPtr
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();

	sl::StdList <axl::io::NetworkAdapterDesc> adapterList;
	size_t adapterCount = axl::io::createNetworkAdapterDescList (&adapterList);

	if (adapterList.isEmpty ())
	{
		if (adapterCountPtr.m_p)
			*(size_t*) adapterCountPtr.m_p = 0;

		if (addressCountPtr.m_p)
			*(size_t*) addressCountPtr.m_p = 0;

		return g_nullPtr;
	}

	ScopedNoCollectRegion noCollectRegion (runtime, false);

	sl::Iterator <axl::io::NetworkAdapterDesc> it = adapterList.getHead ();

	size_t addressCount = 0;
	NetworkAdapterDesc* prevAdapter = NULL;

	DataPtr adapterPtr = createNetworkAdapterDesc (runtime, *it, NULL);
	NetworkAdapterDesc* adapter = (NetworkAdapterDesc*) adapterPtr.m_p;
	addressCount += adapter->m_addressCount;

	DataPtr resultPtr = adapterPtr;

	for (it++; it; it++)
	{
		adapterPtr = createNetworkAdapterDesc (runtime, *it, adapter);
		adapter = (NetworkAdapterDesc*) adapterPtr.m_p;
		addressCount += adapter->m_addressCount;
	}

	if (adapterCountPtr.m_p)
		*(size_t*) adapterCountPtr.m_p = adapterCount;

	if (addressCountPtr.m_p)
		*(size_t*) addressCountPtr.m_p = addressCount;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
