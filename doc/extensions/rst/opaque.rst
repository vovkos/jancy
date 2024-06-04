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

Opaque Classes
==============

When implementing the interaction between your Jancy script and the host C/C++ application you will often need to hide the details of C++ implementation of classes exported to the Jancy namespace. Jancy simplifies the job by providing opaque classes.

.. code-block:: jnc

	opaque class Serial {
		uint_t autoget property m_baudRate;
		SerialFlowControl autoget property m_flowControl;
		uint_t autoget property m_dataBits; // typically 5..8
		SerialStopBits autoget property m_stopBits;
		SerialParity autoget property m_parity;

		// ...

		Serial* operator new();
	}

The corresponding C++ implementation class would look somewhat like this:

.. code-block:: jnc

	class Serial: public jnc::IfaceHdr {
	public:
		JNC_BEGIN_CLASS ("io.Serial", ApiSlot_Serial)
			JNC_AUTOGET_PROPERTY ("m_baudRate",    &Serial::setBaudRate)
			JNC_AUTOGET_PROPERTY ("m_flowControl", &Serial::setFlowControl)
			JNC_AUTOGET_PROPERTY ("m_dataBits",    &Serial::setDataBits)
			JNC_AUTOGET_PROPERTY ("m_stopBits",    &Serial::setStopBits)
			JNC_AUTOGET_PROPERTY ("m_parity",      &Serial::setParity)

			// ...

			JNC_OPERATOR_NEW (&Serial::operatorNew)
		JNC_API_END_CLASS()

		uint_t m_baudRate;
		axl::io::SerialFlowControl m_flowControl;
		uint_t m_dataBits;
		axl::io::SerialStopBits m_stopBits;
		axl::io::SerialParity m_parity;

		// ...

	protected:
		// hidden implementation

		axl::io::Serial m_serial;
		mt::Lock m_ioLock;
		uint_t m_ioFlags;
		IoThread m_ioThread;

	};

Opaque classes can be neither derived from nor allocated statically, on stack, or as a class field member. This is because the Jancy compiler has no information about their full layout -- they are opaque after all.

Opaque classes can only be allocated on the heap and only if their declaration includes **operator new**. The developer can choose which opaque classes should be creatable and which ones should be exposed as non-creatable host interfaces.
