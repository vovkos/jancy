Multicasts and Events
=====================

Multicasts are compiler-generated classes capable of accumulating function pointers and then calling them all at once. The main (but not the only) application of multicasts is the implementation of a publisher-subscriber pattern.

A multicast stores function pointers, so a multicast declaration looks similar to a function pointer declaration (and just like a function pointer it can be thin, fat, or weak).

.. code-block:: none

	multicast m (int);

A multicast class provides the following methods (this is for the multicast from the above example):

.. code-block:: none

	void clear ();
	intptr setup (function* (int)); // returns cookie
	intptr add (function* (int));   // returns cookie
	function* remove (intptr cookie);
	function* getSnapshot ();
	void call (int);

The set() and add() methods return a cookie which can later be used to efficiently remove the function pointer from the multicast.

Some of these methods have operator aliases:

.. code-block:: none

	multicast m ();
	m.setup (foo);     // same as: m = foo;
	m.add (bar);       // same as: m += bar;
	m.remove (cookie); // same as: m -= cookie;
	clear ();          // same as: m = null;

The following example demonstrates some of the basic operations on multicasts:

.. code-block:: none

	foo (int x)
	{
	    // ...
	}

	bar (
	    int x,
	    int y
	    )
	{
	    // ...
	}

	baz ()
	{
	    multicast m (int);
	    intptr fooCookie = m.add (foo); // same as: m += foo;

	    m += bar ~(, 200); // add a pointer with partial application
	    m (100); // => foo (100); bar (100, 200);

	    m -= fooCookie;
	    m (100); // => bar (100, 200);
	    m.clear (); // same as: m = null;

	    // ...
	}

Events are special pointers to multicasts. They restrict access to multicast methods ``call``, ``setup``, and ``clear``.

.. code-block:: none

	foo (int x)
	{
	    // ...
	}

	bar ()
	{
	    multicast m (int);

	    event* p (int) = m;
	    p += foo; // ok
	    p (100);  // error: 'call' is not accessible
	    p.clear ();  // error: 'clear' is not accessible
	}

Declaring a variable or a field with the event type yields a dual access policy. Friends of the namespace have multicast access to it, aliens have event access only. Read more about the dual access control model here.

.. code-block:: none

	class C1
	{
	    bool work ()
	    {
	        // ...

	        m_onComplete (); // ok, friends have multicast access to m_onComplete
	        return true;
	    }

	    event m_onComplete ();
	}

	foo ()
	{
	    // ...
	}

	bar ()
	{
	    C1 c;
	    c.m_onComplete += foo; // ok, aliens have event access to m_onComplete
	    c.work ();

	    c.m_onComplete (); // error: 'call' is not accessible
	}

Converting from a multicast to a function pointer is inherently ambiguous: should the resulting pointer be **live** or **snapshot**? In other words, if after creating a function pointer we modify the multicast, should this function pointer see the changes made to the multicast or not?

To deal with this ambiguity, Jancy multicast classes provide the getSnapshot () method. Casting a multicast to a function pointer implicitly yields a **live** pointer, while the getSnapshot () method returns a snapshot.

.. code-block:: none

	foo ()
	{
	    // ...
	}

	bar ()
	{
	    // ...
	}

	baz ()
	{
	    multicast m () = foo;

	    function* f1 (int) = m;                // live
	    function* f2 (int) = m.getSnapshot (); // obviously, a snapshot

	    // modify multicast

	    m += bar;

	    f1 (); // => foo (); bar ();
	    f2 (); // => foo ();

	    return 0;
	}
