ABI-compatibility with C/C++
============================

After the proper declaration of a data type in the Jancy scripts and in the host C/C++ application it becomes possible to directly pass data through arguments and return values without the need to explicitly push and pop the stack of the virtual machine or pack data into variant-like containers.

The following types are supported:

* All primitive C/C++ types (also integer types with inverted byte order, a.k.a. bigendians)
* Structs (with arbitrary pack factor)
* Unions
* Bit fields (in structs and unions)
* Arrays
* C/C++ data and function pointers

The following calling conventions are supported:

* cdecl (Microsoft/gcc)
* stdcall (Microsoft/gcc)
* Microsoft x64
* System V

The above brings simplicity and effectiveness to the application-script interaction.

Consider the following example of mapping Jancy declarations to C++ implementation:

.. code-block:: none

	opaque class Socket
	{
	    // ...

	    SocketAddress const property m_address;
	    SocketAddress const property m_peerAddress;

	    bool readonly m_isOpen;
	    uint_t m_syncId;

	    Socket* operator new ();

	    bool errorcode open (
	        Protocol protocol,
	        SocketAddress const* address = null
	        );

	    void close ();

	    // ...
	}

The implementation in C/C++ would look something like:

.. code-block:: none

	class Socket: public jnc::IfaceHdr
	{
	public:
	    // conventient macros for name-to-address mapping

	    JNC_BEGIN_CLASS ("io.Socket", ApiSlot_Socket)
	        JNC_OPERATOR_NEW (&Socket::OperatorNew)
	        JNC_CONST_PROPERTY ("m_address",     &Socket::getAddress)
	        JNC_CONST_PROPERTY ("m_peerAddress", &Socket::getPeerAddress)
	        JNC_FUNCTION ("open",     &Socket::open)
	        JNC_FUNCTION ("close",    &Socket::close)

	        // ...
	    JNC_END_CLASS ()

	    // these fields are directly accessed from Jancy

	    bool m_isOpen;
	    uint_t m_syncId;

	    // ...

	    // these methods are directly called from Jancy

	    static
	    Socket*
	    operatorNew ();

	    sockaddr
	    AXL_CDECL
	    getAddress ();

	    sockaddr
	    AXL_CDECL
	    getPeerAddress ();

	    bool
	    AXL_CDECL
	    open (
	        int protocol,
	        jnc::DataPtr addressPtr
	        );

	    void
	    AXL_CDECL
	    close ();

	    // ...
	};

The described compatibility also means you can copy-paste C definitions of communication protocol headers (such as TCP, UDP, etc.). C is the de-facto standard of system programming and it’s possible to find C definition for virtually any protocol in existence. Need to use this protocol from Jancy for analysis, implementation, or testing? Copy-paste the definition of protocol headers into Jancy!

.. code-block:: none

	enum IpProtocol: uint8_t
	{
	    Icmp = 1,
	    Tcp  = 6,
	    Udp  = 17,
	}

	struct IpHdr
	{
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
