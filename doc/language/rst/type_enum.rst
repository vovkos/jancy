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

Enums
=====

Jancy brings a couple of enhancements to the enumeration types as well.

In Jancy, traditional enums conceal member identifiers within their enum namespaces to prevent namespace pollution. Plus, Jancy enums can be derived from an integer type. This comes handy when declaring fields of protocol headers.

.. code-block:: none

	enum IcmpType: uint8_t
	{
	    EchoReply               = 0,
	    DestinationUnreachable  = 3,
	    SourceQuench            = 4,
	    Redirect                = 5,
	    Echo                    = 8,
	    RouterAdvertisement     = 9,
	    RouterSelection         = 10,
	    TimeExceeded            = 11,
	    ParameterProblem        = 12,
	    TimestampRequest        = 13,
	    TimestampReply          = 14,
	    InformationRequest      = 15,
	    InformationReply        = 16,
	    AddressMaskRequest      = 17,
	    AddressMaskReply        = 18,
	    TraceRoute              = 30,
	}

Exposed Enums
-------------

To simplify porting existing C/C++ code into Jancy we offer an **exposed enum** variation, which behaves like a traditional C/C++ enum, i.e. it exposes the member identifier into the parent namespace.

.. code-block:: none

	exposed enum State
	{
	    State_Idle, // = 0
	    State_Connecting,
	    State_Connected,
	    State_Disconnecting,
	}

	foo ()
	{
	    State state = State_Connecting; // State.State_Connecting is also ok
	    state = 100; // error: cast int->enum must be explicit
	}

Bitflag Enums
-------------

Jancy also features **bitflag enums**, which are enumerations dedicated to describing a set of bitflags. A bitflag enum differs from a regular enum in the following aspects:

* It's automatic value assignment sequence is ``1``, ``2``, ``4``, ``8``,... thus describing bit positions
* Bitwise OR operator ``|`` on two operands of matching bitflag enum types yields the same bitflag enum type
* Bitwise AND operator ``&`` on bitflag enum and integer yields the same bitflag enum type
* It's OK to assign ``0`` to a bitflag enum

Like Jancy enums, bitflag enums do not pollute the parent namespace.

.. code-block:: none

	bitflag enum OpenFlags
	{
	    ReadOnly,      // = 0x01
	    Exclusive         = 0x20,
	    DeleteOnClose, // = 0x40
	}

	foo ()
	{
	    OpenFlags flags = 0; // 0 is ok to assign to 'bitflag enum'

	    flags = OpenFlags.ReadOnly | OpenFlags.Exclusive | OpenFlags.DeleteOnClose;
	    flags &= ~OpenFlags.Exclusive;
	    flags = 200; // error: cast int->bitflag enum must be explicit
	}
