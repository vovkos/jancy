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

Function Pointers
=================

Remember nested C language **declarators of death** needed to describe a pointer to a function, which returns a pointer to a function, which returns yet another pointer, and so on?

Nested declarators are **evil**! Fortunately, there are **other ways** to achieve the same result. Jancy uses a different approach, which is much easier to read while allowing to declare function pointers of arbitrary complexity.

.. code-block:: jnc

	foo ()
	{
	    // ...
	}

	int* bar (int x)
	{
	    // ...
	}

	int* function* chooseFunc () (int)
	{
	    return bar;
	}

	main ()
	{
	    function* f () = foo; // boooring!
	    int* function* f2 (int) = bar;
	    int* function* function* f3 () (int) = chooseFunc; // keep going...
	    int* function* function* f4 () (int) = chooseAnotherFunc;
	    int* function* function** f5 [2] () (int) = { &f3, &f4 }; // oh yeah!

	    (*f5 [0]) () (100); // bar (100)
	}

Function pointers can be normal (fat) or ``thin``. ``thin`` pointers are just like C/C++ function pointers: they simply hold the address of the code.

.. code-block:: jnc

	foo (int a)
	{
	    // ...
	}

	bar ()
	{
	    function thin* p (int) = foo;
	    p (10);
	}

Unlike C/C++, the argument conversion is automated (Jancy compiler generates thunks as needed)

.. code-block:: jnc

	foo (int a)
	{
	    // ...
	}

	bar ()
	{
	    typedef FpFunc (double);

	    // explicit cast is required to generate a thunk
	    FpFunc thin* f = (FpFunc thin*) foo;

	    f (3.14);
	}

The true power comes with *fat* function pointers. Besides the code address, fat pointers also hold the address to the closure object, which stores the context captured at the moment of creating the function pointer.

.. code-block:: jnc

	class C1
	{
	    foo ()
	    {
	        // ...
	    }
	}

	bar ()
	{
	    C1 c;

	    function* f () = c.foo; // in this case, pointer to 'c' was captured
	    f ();
	}

Jancy also allows to capture arbitrary arguments in the closure through the use of partial application operator ``~()``

.. code-block:: jnc

	foo (
	    int x,
	    int y
	    )
	{
	    // ...
	}

	bar ()
	{
	    function* f (int) = foo ~(10);
	    f (20); // => foo (10, 20);
	}

You are free to skip arguments during the partial application. For example, you can make it so that the argument 3 comes from the closure, while arguments 1 and 2 come from the call.

.. code-block:: jnc

	class C1
	{
	    foo (
	        int x,
	        int y,
	        int z
	        )
	    {
	        // ...
	    }
	}

	bar ()
	{
	    C1 c;

	    function* f (int, int) = c.foo ~(,, 300);
	    f (100, 200); // => c.foo (100, 200, 300);
	}

Fat function pointers can be ``weak``, meaning they do not retain some of the objects in the closure.

.. code-block:: jnc

	class C1
	{
	    foo (
	        int a,
	        int b,
	        int c
	        )
	    {
	        // ...
	    }
	}

	bar ()
	{
	    C1* c = new C1;

	    function weak* w (int, int) = c.foo (,, 3);

	    // uncomment the next line and C1 will get collected next gc run
	    // c = null;

	    std.collectGarbage ();

	    function* f (int, int) = w;
	    if (f)
	    {
	        // object survived GC run, call it
	        f (1, 2); // c.foo (1, 2, 3);
	    }
	}
