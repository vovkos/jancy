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

Structs/Unions
==============

Jancy supports structs and unions almost the same way C/C++ does. There are certain nuances, though.

Structs/Unions vs Classes
-------------------------

There is a clear distinction between struct/unions and classes in Jancy.

**Structs and unions** represent simple **data blocks**. Variables or fields of struct or union types can be assigned to one another which results in simple ``memcpy``.

More **complex data** structures which *cannot be memcpy-ed* must be represented by :ref:`classes <classes>`.

There are two limitation imposed on structs and unions in Jancy.

1. Structs and unions can have **no virtual** methods.

	Virtual function calls in Jancy (just like in C++, Java and most other OOP languages) are implemented via implicit pointer to ``vtable``, or *virtual method table*. Neither structs nor unions in Jancy have one -- being definitions of simple data blocks, structs and unions only contain the *explicitly declared fields** (and possibly padding to align these fields the same way C/C++ compiler would).

2. Structs and unions can have **no destructors**.

	Firstly, destructors in memcpy-able data structures require extra caution. In C++ this has to be manually handled by the developer via *reference-counting*, *hiding copy-constructors* and assignment-operators or other techniques. But more importantly, *simple* data blocks... well, don't need a destructor! Need to finalize a data structure? Then implement it using classes, not structs or unions!

Unions
------

Unlike in C++, in Jancy it's OK to **derive** from a union:

.. code-block:: jnc

	union MyUnion
	{
	    char m_c;
	    short m_s;
	    int m_i;
	}

	struct MyStruct: MyUnion
	{
	    long m_l;
	}

The layout will be equivalent to the following C++ code snippet:

.. code-block:: jnc

	struct MyUnion
	{
	    union
	    {
	        char m_c;
	        short m_s;
	        int m_i;
	    };
	};

	struct MyStruct: MyUnion
	{
	    long m_l;
	}

Another distinction is needed to prevent unions from being used (intentionally or accidentally) from damaging Jancy meta-data such as virtual function tables or safe pointer validators.

.. note::

	Unions can only containt POD-types.

Let me remind you that POD-types in Jancy are :ref:`primitive types <primitive-types>` and all their aggregations via arrays, structs or unions.

This rule means, that the following code will **not compile**:

.. code-block:: jnc

	class MyClass
	{
		// ...
	}

	union MyUnion
	{
		MyClass m_c;     // <-- error: non-POD
	    char const* m_p; // <-- error: non-POD
	    long m_a [2];
	}
