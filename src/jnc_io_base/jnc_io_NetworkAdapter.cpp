#include "pch.h"
#include "jnc_io_NetworkAdapter.h"

namespace jnc {
namespace io {

//.............................................................................
	
rt::DataPtr
createNetworkAdapterAddress (
	rt::Runtime* runtime,
	const axl::io::NetworkAdapterAddress* srcAddress,
	NetworkAdapterAddress* prevAddress
	)
{
	rt::DataPtr addressPtr = rt::createData <NetworkAdapterAddress> (runtime);
	NetworkAdapterAddress* address = (NetworkAdapterAddress*) addressPtr.m_p;
	address->m_address = srcAddress->m_address;
	address->m_netMaskBitCount = srcAddress->m_netMaskBitCount;

	if (prevAddress)
		prevAddress->m_nextPtr = addressPtr;

	return addressPtr;
}

rt::DataPtr
createNetworkAdapterDesc (
	rt::Runtime* runtime,
	const axl::io::NetworkAdapterDesc* srcAdapter,
	NetworkAdapterDesc* prevAdapter
	)
{
	rt::DataPtr adapterPtr = rt::createData <NetworkAdapterDesc> (runtime);
	NetworkAdapterDesc* adapter = (NetworkAdapterDesc*) adapterPtr.m_p;
	adapter->m_type = srcAdapter->getType ();
	adapter->m_flags = srcAdapter->getFlags ();
	adapter->m_namePtr = rt::strDup (srcAdapter->getName ());
	adapter->m_descriptionPtr = rt::strDup (srcAdapter->getDescription ());
	memcpy (adapter->m_mac, srcAdapter->getMac (), 6);

	if (prevAdapter)
		prevAdapter->m_nextPtr = adapterPtr;

	sl::ConstList <axl::io::NetworkAdapterAddress> addressList = srcAdapter->getAddressList ();
	if (addressList.isEmpty ())
		return adapterPtr;

	sl::Iterator <axl::io::NetworkAdapterAddress> it = addressList.getHead ();
	
	rt::DataPtr addressPtr = createNetworkAdapterAddress (runtime, *it, NULL);
	adapter->m_addressPtr = addressPtr;
	adapter->m_addressCount = addressList.getCount ();

	for (it++; it; it++)
		addressPtr = createNetworkAdapterAddress (runtime, *it, (NetworkAdapterAddress*) addressPtr.m_p);

	return adapterPtr;
}

rt::DataPtr
createNetworkAdapterDescList (
	rt::DataPtr adapterCountPtr,
	rt::DataPtr addressCountPtr	
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();

	sl::StdList <axl::io::NetworkAdapterDesc> adapterList;
	size_t adapterCount = axl::io::createNetworkAdapterDescList (&adapterList);

	if (adapterList.isEmpty ())
	{
		if (adapterCountPtr.m_p)
			*(size_t*) adapterCountPtr.m_p = 0;

		if (addressCountPtr.m_p)
			*(size_t*) addressCountPtr.m_p = 0;

		return rt::g_nullPtr;
	}

	rt::ScopedNoCollectRegion noCollectRegion (runtime, false);

	sl::Iterator <axl::io::NetworkAdapterDesc> it = adapterList.getHead ();

	size_t addressCount = 0;
	NetworkAdapterDesc* prevAdapter = NULL;

	rt::DataPtr adapterPtr = createNetworkAdapterDesc (runtime, *it, NULL);
	NetworkAdapterDesc* adapter = (NetworkAdapterDesc*) adapterPtr.m_p;
	addressCount += adapter->m_addressCount;

	rt::DataPtr resultPtr = adapterPtr;

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

//.............................................................................

} // namespace io
} // namespace jnc
