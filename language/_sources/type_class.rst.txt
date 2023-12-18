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

.. _classes:

Classes
=======

Classes in Jancy are special types of data with ancillary header holding the type information, virtual table pointer, etc. Jancy Classes are what would have been called *reference-types* in C# or Java -- they can *only* be accessed by reference.You **cannot assign** varibles or fields of class types. If you need to clone or copy an instance of a class -- this class should provide a corresponding ``clone`` or ``copy`` method.

Member Access Control
---------------------

There are only 2 access specifiers in Jancy: ``public`` and ``protected`` (read more about Jancy :ref:`dual modifiers <dual-modifiers>`).

Both Java and C++ styles of declaring member access are supported. Contrary to most modern languages, the default access mode is public.

.. code-block:: jnc

	class C1
	{
	    int m_x; // public by default

	protected: // C++-style of access specification
	    int m_y;

	    public int m_z; // Java-style of access specification

	    // ...
	}

Method Body Placement
---------------------

Jancy supports both Java and C++ styles of placing method bodies: it is up to developer to choose whether this will be **in-class** or **out-of-class** (i.e. in a different compilation unit).

.. code-block:: jnc

	class C1
	{
	    foo (); // C++-style

	    bar () // Java-style
	    {
	        // ...
	    }
	}

	// out-of-class method definition

	C1.foo ()
	{
	    // ...
	}

Construction/Destruction
------------------------

Jancy supports in-place field initializers, constructors, destructors, static constructors, static destructors, and preconstructors. Constructors can be overloaded, the rest of construction methods must have no arguments.

Preconstructors are special methods that will be called before any of the overloaded constructors (similar to Java initializer blocks).

Constructor and destructor syntax is a bit different from most languages, as Jancy uses explicit keywords.

.. code-block:: jnc

	class C1
	{
	    int m_x = 100; // in-place initializer

	    static construct ();
	    static destruct ();

	    preconstruct ();  // will be called before every overloaded constructor

	    construct ();
	    construct (int x);
	    construct (double x);

	    destruct ();

	    // ...
	}

	// out-of-class method definitions

	C1.static construct ()
	{
	    // ...
	}

	// ...

Jancy has pointers and, contrary to most managed languages, has no distinction between value-types and reference-types.

What is a pointer, must look like a pointer.

A type of a class variable or a field does not get implicitly converted to a class pointer type. Like in C++, the declaration of a class variable or field is an instruction to allocate a new object.

Member class fields get allocated on a parent memory block, global class variables get station allocation, local class variables are allocated on heap (unless explicitly specified otherwise).

.. code-block:: jnc

	class C1
	{
	    // ...
	}

	class C2
	{
	    // ...

	    C1 m_classField; // allocated as part of C2 layout
	}

	C2 g_classVariable; // allocated statically

	foo ()
	{
	    C1 a;        // allocated on heap (same as: C1* a = heap new C1;)
	    stack C1 b;  // allocated on stack (same as: C1* b = stack new C1;)
	    static C2 c; // allocated statically (same as: C1* c = static new C1;)
	    thread C2 d; // error: thread-local variable cannot be of class type
	    thread C2* e = new C2; // OK

	    // ...
	}

	// ...

Jancy has a small syntactic difference with regard to calling a constructor of a class variable or field. This is to address an inherent ambiguity of the C/C++ constructor invocation syntax:

.. code-block:: jnc

	C1 a (); // is it a function 'a' which returns C1?
	         //     or a construction of variable 'a' of type C1?

This ambiguity is even trickier to handle in Jancy given the fact that Jancy does not enforce the **declaration-before-usage** paradigm. To counter the ambiguity, Jancy introduces a slight syntax modification which fully resolves the issue:

.. code-block:: jnc

	class C1
	{
	    construct ();
	    construct (int x);

	    // ...
	}

	C1 g_a construct ();
	C1 g_b construct (10);

	// with operator new there is no ambiguity, so both versions of syntax are OK

	C1* g_c = new C1 construct (20);
	C1* g_d = new C1 (30);

Operator Overloading
--------------------

Jancy supports operator overloading. Like in C++, any unary, binary, cast or call operators can be overloaded.

.. code-block:: jnc

	class C1
	{
	    operator += (int d) // overloaded '+=' operator
	    {
	        // ...
	    }
	}

	foo ()
	{
	    C1 c;
	    c += 10;

	    // ...
	}

Multiple Inheritance
--------------------

Jancy uses a simple multiple inheritance model (multiple instances of shared bases -- if any). The infamous virtual multiple inheritance model of C++ is not and will not be supported.

Multiple inheritance is an extremely useful and unfairly abandoned tool, which allows the most natural sharing of interface implementation.

Virtual methods are declared using keywords ``virtual``, ``abstract``, and ``override``.

.. code-block:: jnc

	class I1
	{
	    abstract foo ();
	}

	class C1: I1
	{
	    override foo ()
	    {
	        // ...
	    }
	}

	class I2
	{
	    abstract bar ();

	    abstract baz (
	        int x,
	        int y
	        );
	}

	class C2: I2
	{
	    override baz (
	        int x,
	        int y
	        )
	    {
	        // ...
	    }
	}

	struct Point
	{
	    int m_x;
	    int m_y;
	}

	class C3:
	    C1,
	    C2,
	    Point // it's ok to inherit from structs and even unions
	{
	    override baz (
	        int x,
	        int y
	        );
	}

	// it's ok to use storage specifier in out-of-class definition by the way.
	// (must match the original one, of course)

	override C3.baz (
	    int x,
	    int y
	    )
	{
	    // ...
	}

Jancy provides keywords ``basetype`` and ``basetype1`` .. ``basetype9`` to conveniently reference base types for construction or namespace resolution.

.. code-block:: jnc

	class Base1
	{
	    construct (
	        int x,
	        int y
	        );

	    foo ();
	}

	class Base2
	{
	    construct (int x);

	    foo ();
	}

	class Derived:
	    Base1,
	    Base2
	{
	    construct (
	        int x,
	        int y,
	        int z
	        )
	    {
	        basetype1.construct (x, y);
	        basetype2.construct (z);

	        // ...
	    }

	    foo ()
	    {
	        basetype1.foo ();

	        // ...
	    }
	}
