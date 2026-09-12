.. .............................................................................
..
..  This file is part of the Jancy toolkit.
..
..  Jancy is distributed under the MIT license.
..  For details see accompanying license.txt file,
..  the public copy of which is also available at:
..  http://tibbo.com/downloads/archive/jancy/license.txt
..
.. .............................................................................

Compatibility with C
====================

After the proper declarations in the Jancy scripts and in the host C/C++ application it becomes possible to **directly pass data** through arguments and return values without the need to explicitly push and pop the stack of the virtual machine or pack data into variant-like containers.

The following types are supported:

* All primitive C/C++ types (also integer types with inverted byte order, a.k.a. **bigendians**)
* Structs (with arbitrary pack factor)
* Unions
* Bit fields (both in structs and unions)
* Arrays
* C/C++ data and function pointers

The following calling conventions are supported:

* cdecl (Microsoft/gcc)
* stdcall (Microsoft/gcc)
* Microsoft x64
* System V

The above brings **simplicity and effectiveness** to the application-script interaction.

Consider the following example of mapping Jancy declarations to C++ implementation:

.. code-block:: jnc

	opaque class Socket {
		SocketAddress const property m_address;
		SocketAddress const property m_peerAddress;

		bool readonly m_isOpen;
		uint_t m_syncId;
		event m_onSocketEvent (SocketEventParams const* params);
		// ...

		bool errorcode open(
			Protocol protocol,
			SocketAddress const* address = null
		);

		void close();
		// ...
	}

The implementation in C++ would look something like:

.. code-block:: cpp

	class Socket: public jnc::IfaceHdr {
	public:
		// these fields are directly accessed from Jancy
		bool m_isOpen;
		uint_t m_syncId;
		ClassBox <Multicast> m_onSocketEvent;
		// ...

	protected:
		// these fields are not accessible from Jancy
		sys::Lock m_ioLock;
		volatile uint_t m_ioFlags;
		IoThread m_ioThread;
		// ...

	public:
		// these methods are directly called from Jancy

		sockaddr
		AXL_CDECL
		getAddress();

		sockaddr
		AXL_CDECL
		getPeerAddress();

		bool
		AXL_CDECL
		open(
			int protocol,
			jnc::DataPtr addressPtr
		);

		void
		AXL_CDECL
		close();

		// ...
	};

C++ implementation must be **mapped** to Jancy names so Jancy knows where to find particular methods:

.. code-block:: cpp

	JNC_DEFINE_OPAQUE_CLASS_TYPE (
		Socket,
		"io.Socket",
		g_ioLibGuid,
		IoLibCacheSlot_Socket,
		Socket,
		NULL
		)

	// function map table for the Socket type

	JNC_BEGIN_TYPE_FUNCTION_MAP(Socket)
		JNC_MAP_CONSTRUCTOR(&jnc::construct<Socket>)
		JNC_MAP_DESTRUCTOR(&jnc::destruct<Socket>)
		JNC_MAP_CONST_PROPERTY("m_address",      &Socket::getAddress)
		JNC_MAP_CONST_PROPERTY("m_peerAddress",  &Socket::getPeerAddress)
		// ...
	JNC_END_TYPE_FUNCTION_MAP()

That's it. Now Jancy can call C++ methods directly.

Source Compatibility
--------------------

The compatibility between Jancy and C doesn't end at ABI-level. High level of source compatibility between the two languages means that you can often times simply **copy-paste** C algorithms, definitions of communication protocol headers, etc.

C is the de-facto standard of system programming. It's possible to find C definition for virtually any algorithm or protocol in existence. Let, say, you need to use some protocol headers from Jancy. Simply copy-paste the C declarations and often times it will compile without even touching a thing! It is, however, recommended to *make adjustments* -- just so you can make use of Jancy features such as **derived enums** or **bigendians**:

.. code-block:: jnc

	enum IpProtocol: uint8_t {
		Icmp = 1,
		Tcp  = 6,
		Udp  = 17,
	}

	struct IpHdr {
		uint8_t m_headerLength : 4;
		uint8_t m_version      : 4;
		uint8_t m_typeOfService;
		bigendian uint16_t m_totalLength;
		uint16_t m_identification;
		uint16_t m_flags;
		uint8_t m_timeToLive;
		IpProtocol m_protocol;
		bigendian uint16_t m_headerChecksum;
		uint32_t m_srcAddress;
		uint32_t m_dstAddress;
	}
