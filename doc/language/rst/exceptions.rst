Exception Handling
==================

Jancy exceptions handling model applies a layer of syntactic sugar over good old C-style error code checking. As a result, it is extremely transparent and easy to support from the host C/C++ application.

A function marked by the ``errorcode`` modifier will have its return value interpreted as an error code. Intuitive defaults are assumed: ``false`` for bools, ``-1`` for integers integers and ``null`` for pointers.

.. code-block:: none

	bool errorcode foo (int a)
	{
	    printf ("foo (%d)\n", a);
	    return a > 0;
	}

If return values match, the error code is automatically propagated:

.. code-block:: none

	int errorcode foo (int a);

	int errorcode bar (int a)
	{
	    // ...

	    baz (a);

	    // ...
	}

The ``try`` operator shields an expression from **throwing**:

.. code-block:: none

	int result = try baz (-5);

The ``try`` block shields a parent scope from **throwing** even if this parent scope has no ``catch``:

.. code-block:: none

	foo ()
	{
	    // ...

	    try
	    {
	        baz (20);
	        baz (-1);
	        baz (21); // never get here
	    }

	    // ...
	}

``catch`` and ``finally`` can be within any scope:

.. code-block:: none

	int errorcode bar (int a)
	{
	    // ...

	catch:
	    printf ("bar.catch\n");
	    return -5;

	finally:
	    printf ("bar.finally\n");
	}

When calling a function, the developer can use either an error code check or exception semantics depending on what's more appropriate or convenient in each particular case.

.. code-block:: none

	int main ()
	{
	    // ...

	    int result = try bar ();
	    if (result < 0)
	    {
	        // handle error
	    }
	}
