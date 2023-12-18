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

once
====

Jancy provides an elegant syntax for lazy initialization. Prefix the necessary piece of code with ``once`` and the compiler will generate a thread-safe wrapper. The latter will ensure that this code executes once per each program run.

.. code-block:: jnc

	foo ()
	{
	    once initialize ();

	    // ...
	}

If your lazy initialization requires more than a single statement, enclose the entire block of your initialization code in a compound statement:

.. code-block:: jnc

	foo ()
	{
	    once
	    {
	        initializeTables ();
	        initializeMaps ();
	        initializeIo ();

	        // ...
	    }

	    // ...
	}

Jancy also provides a way to run the lazy initialization once per thread. Use **threadlocal once** to achieve this:

.. code-block:: jnc

	foo ()
	{
	    threadlocal once initializeThread ();

	    // ...
	}
