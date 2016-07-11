#pragma once

#ifdef _JNC_IO_BASE
#	include "../jnc_io_base/jnc_io_IoLibGlobals.h"
#endif

namespace jnc {
namespace io {

//.............................................................................

enum AddressFamily
{
	AddressFamily_Undefined = 0,
	AddressFamily_Unused    = 1,
	AddressFamily_Ip4       = 2, // AF_INET
	AddressFamily_Ip6, // can be different
	AddressFamily__Count,
};

//.............................................................................

struct Address_ip4: public in_addr
{
#ifdef _JNC_IO_BASE
	JNC_BEGIN_TYPE_MAP ("io.Address_ip4", g_ioLibCacheSlot, IoLibTypeCacheSlot_Address_ip4)
		JNC_MAP_FUNCTION ("parse",     &Address_ip4::parse)
		JNC_MAP_FUNCTION ("getString", &Address_ip4::getString)
	JNC_END_CLASS_TYPE_MAP ()
#endif // _JNC_IO_BASE

	static 
	bool
	parse (
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseAddr_ip4 (
			(in_addr*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	DataPtr
	getString (DataPtr selfPtr)
	{
		sl::String string = axl::io::getAddrString_ip4 ((const in_addr*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Address_ip6: public in6_addr
{
#ifdef _JNC_IO_BASE
	JNC_BEGIN_TYPE_MAP ("io.Address_ip6", g_ioLibCacheSlot, IoLibTypeCacheSlot_Address_ip6)
		JNC_MAP_FUNCTION ("parse",     &Address_ip6::parse)
		JNC_MAP_FUNCTION ("getString", &Address_ip6::getString)
	JNC_END_CLASS_TYPE_MAP ()
#endif // _JNC_IO_BASE
	
	static 
	bool
	parse (
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseAddr_ip6 (
			(in6_addr*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	DataPtr
	getString (DataPtr selfPtr)
	{
		sl::String string = axl::io::getAddrString_ip6 ((const in6_addr*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//.............................................................................

struct SocketAddress_ip4: public sockaddr_in
{
#ifdef _JNC_IO_BASE
	JNC_BEGIN_TYPE_MAP ("io.SocketAddress_ip4", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddress_ip4)
		JNC_MAP_FUNCTION ("isEqual",   &SocketAddress_ip4::isEqual)
		JNC_MAP_FUNCTION ("isMatch",   &SocketAddress_ip4::isMatch)
		JNC_MAP_FUNCTION ("parse",     &SocketAddress_ip4::parse)
		JNC_MAP_FUNCTION ("getString", &SocketAddress_ip4::getString)
	JNC_END_CLASS_TYPE_MAP ()
#endif // _JNC_IO_BASE

	static 
	bool
	isEqual (
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrEqual_ip4 (
			(const sockaddr_in*) selfPtr.m_p, 
			(const sockaddr_in*) addressPtr.m_p
			);
	}

	static 
	bool
	isMatch (
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrMatch_ip4 (
			(const sockaddr_in*) selfPtr.m_p, 
			(const sockaddr_in*) addressPtr.m_p
			);
	}

	static 
	bool
	parse (
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseSockAddr_ip4 (
			(sockaddr_in*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	DataPtr
	getString (DataPtr selfPtr)
	{
		sl::String string = axl::io::getSockAddrString_ip4 ((const sockaddr_in*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress_ip6: public sockaddr_in6
{
#ifdef _JNC_IO_BASE
	JNC_BEGIN_TYPE_MAP ("io.SocketAddress_ip6", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddress_ip6)
		JNC_MAP_FUNCTION ("isEqual",   &SocketAddress_ip6::isEqual)
		JNC_MAP_FUNCTION ("isMatch",   &SocketAddress_ip6::isMatch)
		JNC_MAP_FUNCTION ("parse",     &SocketAddress_ip6::parse)
		JNC_MAP_FUNCTION ("getString", &SocketAddress_ip6::getString)
	JNC_END_CLASS_TYPE_MAP ()
#endif // _JNC_IO_BASE
	
	static 
	bool
	isEqual (
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrEqual_ip6 (
			(const sockaddr_in6*) selfPtr.m_p, 
			(const sockaddr_in6*) addressPtr.m_p
			);
	}

	static 
	bool
	isMatch (
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrMatch_ip6 (
			(const sockaddr_in6*) selfPtr.m_p, 
			(const sockaddr_in6*) addressPtr.m_p
			);
	}

	static 
	bool
	parse (
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseSockAddr_ip6 (
			(sockaddr_in6*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	DataPtr
	getString (DataPtr selfPtr)
	{
		sl::String string = axl::io::getSockAddrString_ip6 ((const sockaddr_in6*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress
{
#ifdef _JNC_IO_BASE
	JNC_BEGIN_TYPE_MAP ("io.SocketAddress", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddress)
		JNC_MAP_FUNCTION ("isEqual",   &SocketAddress::isEqual)
		JNC_MAP_FUNCTION ("isMatch",   &SocketAddress::isMatch)
		JNC_MAP_FUNCTION ("parse",     &SocketAddress::parse)
		JNC_MAP_FUNCTION ("getString", &SocketAddress::getString)
	JNC_END_CLASS_TYPE_MAP ()
#endif // _JNC_IO_BASE
	
	union
	{
		uint16_t m_family;
		sockaddr_in m_addr_ip4;
		sockaddr_in6 m_addr_ip6;		
	};

	static 
	bool
	isEqual (
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return ((SocketAddress*) selfPtr.m_p)->getSockAddr ().isEqual ((const sockaddr*) addressPtr.m_p);
	}

	static 
	bool
	isMatch (
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return ((SocketAddress*) selfPtr.m_p)->getSockAddr ().isMatch ((const sockaddr*) addressPtr.m_p);
	}

	static 
	bool
	parse (
		DataPtr selfPtr,
		DataPtr stringPtr
		);

	static 
	DataPtr
	getString (DataPtr selfPtr)
	{
		sl::String string = ((SocketAddress*) selfPtr.m_p)->getSockAddr ().getString ();
		return rt::strDup (string, string.getLength ());
	}

	axl::io::SockAddr
	getSockAddr () const;

	void
	setSockAddr (const axl::io::SockAddr& sockAddr);

	static
	SocketAddress
	fromSockAddr (const axl::io::SockAddr& sockAddr);
};

//.............................................................................

} // namespace io
} // namespace jnc
