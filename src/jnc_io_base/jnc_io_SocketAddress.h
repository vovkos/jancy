#pragma once

#include "jnc_io_IoLibGlobals.h"

namespace jnc {
namespace io {

//.............................................................................

struct Address_ip4: public in_addr
{
	JNC_BEGIN_TYPE_MAP ("io.Address_ip4", g_ioLibCacheSlot, IoLibTypeCacheSlot_Address_ip4)
		JNC_MAP_FUNCTION ("parse",     &Address_ip4::parse)
		JNC_MAP_FUNCTION ("getString", &Address_ip4::getString)
	JNC_END_CLASS_TYPE_MAP ()

	static 
	bool
	parse (
		rt::DataPtr selfPtr,
		rt::DataPtr stringPtr
		)
	{
		return axl::io::parseAddr_ip4 (
			(in_addr*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	rt::DataPtr
	getString (rt::DataPtr selfPtr)
	{
		sl::String string = axl::io::getAddrString_ip4 ((const in_addr*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Address_ip6: public in_addr
{
	JNC_BEGIN_TYPE_MAP ("io.Address_ip6", g_ioLibCacheSlot, IoLibTypeCacheSlot_Address_ip6)
		JNC_MAP_FUNCTION ("parse",     &Address_ip6::parse)
		JNC_MAP_FUNCTION ("getString", &Address_ip6::getString)
	JNC_END_CLASS_TYPE_MAP ()
	
	static 
	bool
	parse (
		rt::DataPtr selfPtr,
		rt::DataPtr stringPtr
		)
	{
		return axl::io::parseAddr_ip6 (
			(in6_addr*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	rt::DataPtr
	getString (rt::DataPtr selfPtr)
	{
		sl::String string = axl::io::getAddrString_ip6 ((const in6_addr*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//.............................................................................

struct SocketAddress_ip4: public in_addr
{
	JNC_BEGIN_TYPE_MAP ("io.SocketAddress_ip4", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddress_ip4)
		JNC_MAP_FUNCTION ("isEqual",   &SocketAddress_ip4::isEqual)
		JNC_MAP_FUNCTION ("isMatch",   &SocketAddress_ip4::isMatch)
		JNC_MAP_FUNCTION ("parse",     &SocketAddress_ip4::parse)
		JNC_MAP_FUNCTION ("getString", &SocketAddress_ip4::getString)
	JNC_END_CLASS_TYPE_MAP ()

	static 
	bool
	isEqual (
		rt::DataPtr selfPtr,
		rt::DataPtr addressPtr
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
		rt::DataPtr selfPtr,
		rt::DataPtr addressPtr
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
		rt::DataPtr selfPtr,
		rt::DataPtr stringPtr
		)
	{
		return axl::io::parseSockAddr_ip4 (
			(sockaddr_in*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	rt::DataPtr
	getString (rt::DataPtr selfPtr)
	{
		sl::String string = axl::io::getSockAddrString_ip4 ((const sockaddr_in*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress_ip6: public in_addr
{
	JNC_BEGIN_TYPE_MAP ("io.SocketAddress_ip6", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddress_ip6)
		JNC_MAP_FUNCTION ("isEqual",   &SocketAddress_ip6::isEqual)
		JNC_MAP_FUNCTION ("isMatch",   &SocketAddress_ip6::isMatch)
		JNC_MAP_FUNCTION ("parse",     &SocketAddress_ip6::parse)
		JNC_MAP_FUNCTION ("getString", &SocketAddress_ip6::getString)
	JNC_END_CLASS_TYPE_MAP ()
	
	static 
	bool
	isEqual (
		rt::DataPtr selfPtr,
		rt::DataPtr addressPtr
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
		rt::DataPtr selfPtr,
		rt::DataPtr addressPtr
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
		rt::DataPtr selfPtr,
		rt::DataPtr stringPtr
		)
	{
		return axl::io::parseSockAddr_ip6 (
			(sockaddr_in6*) selfPtr.m_p, 
			(const char*) stringPtr.m_p
			);
	}

	static 
	rt::DataPtr
	getString (rt::DataPtr selfPtr)
	{
		sl::String string = axl::io::getSockAddrString_ip6 ((const sockaddr_in6*) selfPtr.m_p);
		return rt::strDup (string, string.getLength ());
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress: public in_addr
{
	JNC_BEGIN_TYPE_MAP ("io.SocketAddress", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddress)
		JNC_MAP_FUNCTION ("isEqual",   &SocketAddress::isEqual)
		JNC_MAP_FUNCTION ("isMatch",   &SocketAddress::isMatch)
		JNC_MAP_FUNCTION ("parse",     &SocketAddress::parse)
		JNC_MAP_FUNCTION ("getString", &SocketAddress::getString)
	JNC_END_CLASS_TYPE_MAP ()
	
	static 
	bool
	isEqual (
		rt::DataPtr selfPtr,
		rt::DataPtr addressPtr
		)
	{
		return ((axl::io::SockAddr*) selfPtr.m_p)->isEqual ((const sockaddr*) addressPtr.m_p);
	}

	static 
	bool
	isMatch (
		rt::DataPtr selfPtr,
		rt::DataPtr addressPtr
		)
	{
		return ((axl::io::SockAddr*) selfPtr.m_p)->isMatch ((const sockaddr*) addressPtr.m_p);
	}

	static 
	bool
	parse (
		rt::DataPtr selfPtr,
		rt::DataPtr stringPtr
		)
	{
		return ((axl::io::SockAddr*) selfPtr.m_p)->parse ((const char*) stringPtr.m_p);
	}

	static 
	rt::DataPtr
	getString (rt::DataPtr selfPtr)
	{
		sl::String string = ((const axl::io::SockAddr*) selfPtr.m_p)->getString ();
		return rt::strDup (string, string.getLength ());
	}
};

//.............................................................................

} // namespace io
} // namespace jnc
