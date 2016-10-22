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

Data Pointers
=============

Do we even need data pointers? In C/C++ world that's not much of a question: using pointers is the one and only way of working with dynamic memory. What about the managed world, do we need data pointers there?

Most managed language designers believe that the answer is NO. This is largely because data pointers fall into the disadvantageous area on the risk/reward chart for most programming tasks. There is, however, one programming area where the use of data pointers truly shines: working with binary data.

Here is an example. Try to write Java code that deconstructs an Ethernet packet. Compare the resulting mess of fixed index array references and word types assembled from bytes to the clean and efficient code in C that will superimpose protocol header structs on the buffer data, then traverse the packet using pointer arithmetic!

Since Jancy was conceived as the scripting language for our IO Ninja software, living without pointers was out of the question. Instead, we made data pointers safe. Safe data pointers and safe pointer arithmetic are among the biggest innovations of Jancy.

As with many languages, the Jancy runtime doesn't allow access to data via a pointer that failed the range check. Unfortunately, range checks are not enough for stack data pointers:

.. code-block:: jnc

	foo ()
	{
	    //...

	    int* p;

	    {
	        int a = 10;
	        p = &a;
	    }

	    int b = 20;

	    *p = 30; // oh-oh

	    //...
	}

Pointer ``p`` obviously passes the range check (it has not been changed!) but accessing this pointer will write to the dead and, possibly, re-allocated location. That happens because stack pointers become invalid even without modification, simply by running out of scope.

To address this issue, Jancy pointer validators also maintain integer thread-local variable holding the target scope level. The Jancy runtime prevents storing an address with a higher scope level at the location with a lower scope level.

Simply put, the approach used by Jancy is this: check the data range at the pointer access, check the scope level at the pointer assignment.

.. code-block:: jnc

	int* g_p;

	foo ()
	{
	    int* p = null;

	    int x = *p; // error: pointer out of range

	    int a [] = { 10, 20, 30 };
	    g_p = a;    // error: storing pointer in location with lesser scope level

	    int** p2 = &g_p;
	    *p2 = &x;   // error: storing pointer in location with lesser scope level

	    int i = countof (a);
	    x = a [i];  // error: pointer out of range
	}

Our safe pointers are not thread-safe. It's still possible to corrupt a pointer validator in a multi-threaded environment. Still, our solution covers a lot of bases and future Jancy versions will likely address the issue by preventing race conditions on pointer validators.

Besides the normal data pointer with validators (**fat** or **safe** data pointers) Jancy also supports **thin** data pointers, which only hold the target address. This might be useful when writing performance-critical code, or for interoperability with the host C/C++ program. **Thin** pointers are not safe.
