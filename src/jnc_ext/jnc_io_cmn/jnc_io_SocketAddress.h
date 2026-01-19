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

JNC_DECLARE_TYPE(Address_ip4)
JNC_DECLARE_TYPE(Address_ip6)
JNC_DECLARE_TYPE(SocketAddress_ip4)
JNC_DECLARE_TYPE(SocketAddress_ip6)
JNC_DECLARE_TYPE(SocketAddress)

//..............................................................................

enum AddressFamily {
	AddressFamily_Undefined = 0,
	AddressFamily_Unused    = 1,
	AddressFamily_Ip4       = 2, // AF_INET
	AddressFamily_Ip6,           // actual value is platform-specific
	AddressFamily__Count,
};

//..............................................................................

struct Address_ip4: public in_addr {
	JNC_DECLARE_TYPE_STATIC_METHODS(Address_ip4)

	bool
	JNC_CDECL
	parse(String string) {
		return axl::io::parseAddr_ip4(this, string >> toAxl);
	}

	static
	String
	getString(Address_ip4* self) {
		return allocateString(axl::io::getAddrString_ip4(self));
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Address_ip6: public in6_addr {
	JNC_DECLARE_TYPE_STATIC_METHODS(Address_ip6)

	bool
	JNC_CDECL
	parse(String string) {
		return axl::io::parseAddr_ip6(this, string >> toAxl);
	}

	static
	String
	getString(Address_ip6* self) {
		return allocateString(axl::io::getAddrString_ip6(self));
	}
};

//..............................................................................

struct SocketAddress_ip4: public sockaddr_in {
	JNC_DECLARE_TYPE_STATIC_METHODS(SocketAddress_ip4)

	bool
	JNC_CDECL
	isEqual(DataPtr addressPtr) {
		return axl::io::isSockAddrEqual_ip4(this, (sockaddr_in*)addressPtr.m_p);
	}

	bool
	JNC_CDECL
	isMatch(DataPtr addressPtr) {
		return axl::io::isSockAddrMatch_ip4(this, (sockaddr_in*)addressPtr.m_p);
	}

	bool
	JNC_CDECL
	parse(String string) {
		return axl::io::parseSockAddr_ip4(this, string >> toAxl);
	}

	static
	String
	getString(SocketAddress_ip4* self) {
		return allocateString(axl::io::getSockAddrString_ip4(self));
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress_ip6: public sockaddr_in6 {
	JNC_DECLARE_TYPE_STATIC_METHODS(SocketAddress_ip6)

	bool
	JNC_CDECL
	isEqual(DataPtr addressPtr) {
		return axl::io::isSockAddrEqual_ip6(this, (sockaddr_in6*)addressPtr.m_p);
	}

	bool
	JNC_CDECL
	isMatch(DataPtr addressPtr) {
		return axl::io::isSockAddrMatch_ip6(this, (sockaddr_in6*)addressPtr.m_p);
	}

	bool
	JNC_CDECL
	parse(String string) {
		return axl::io::parseSockAddr_ip6(this, string >> toAxl);
	}

	static
	String
	getString(SocketAddress_ip6* self) {
		return allocateString(axl::io::getSockAddrString_ip6(self));
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress {
	JNC_DECLARE_TYPE_STATIC_METHODS(SocketAddress)

	union {
		struct {
			uint16_t m_family;
			uint16_t m_port;
		};

		sockaddr_in m_addr_ip4;
		sockaddr_in6 m_addr_ip6;
	};

	bool
	JNC_CDECL
	isEqual(DataPtr addressPtr) {
		return getSockAddr().isEqual(((SocketAddress*)addressPtr.m_p)->getSockAddr());
	}

	bool
	JNC_CDECL
	isMatch(DataPtr addressPtr) {
		return getSockAddr().isMatch(((SocketAddress*)addressPtr.m_p)->getSockAddr());
	}

	bool
	JNC_CDECL
	parse(String string);

	static
	String
	getString(SocketAddress* self) {
		return allocateString(self->getSockAddr().getString());
	}

	axl::io::SockAddr
	getSockAddr() const;

	void
	setSockAddr(const axl::io::SockAddr& sockAddr);

	static
	SocketAddress
	fromSockAddr(const axl::io::SockAddr& sockAddr);
};

//..............................................................................

} // namespace io
} // namespace jnc
