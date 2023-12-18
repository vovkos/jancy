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

.. _disposable:

'disposable' Variables
======================

A lot of classes in Jancy standard library (e.g. `io.File`, `io.Serial`, etc.) provide ``close`` methods to terminate the current session and release associated resources.

If an instance of such class is being abandoned without having its ``close`` method called, it will still be called automatically in the class *destructor* -- which will happen whenever the Jancy Garbage Collector decides to sweep unused objects.

However, leaving this at Garbage Collector' discretion is undesirable, as it leads to undeterministic resource release -- file handle or other resources will be held for an unknown period of time.

An easy way for overcoming this problem is to work with objects that do require a timely invokation of ``close`` (or similar resource-releasing method) using the Jancy ``disposable`` storage specifier.

.. rubric:: Example:

.. ref-code-block:: jnc

	foo ()
	{
	    // ...

	    disposable `io.File` file;
	    file.open ();

	    // work with file...
	} // <-- file.close () is guaranteed to be called upon exiting the scope

When program runs out of scope where a variable of disposable class was declared in, Jancy compiler will insert a call to ``dispose`` method (which is usually *aliased* to an actual release methods such as ``close``). This method will be called no matter how the program runs out of scope, be it a normal control flow, ``return``,  ``break``, ``continue`` or an exception.

You can easily write disposable classes yourself. All you have to do is to provide a ``dispose`` method:

.. ref-code-block:: jnc

	class MyDisposableClass
	{
	    dispose ()
	    {
	        printf ("Releasing resources...");
	    }
	}

	foo ()
	{
	    // ...

	    disposable MyDisposableClass c;

	    // work with c...

	    throw; // <-- c.dispose () will get called
	}

Often times, a class already has a function to release resources (called ``close``, ``release``, etc.); renaming it to ``dispose`` may be undesireable. In such case, you cane make such a class disposable by *aliasing* ``dispose``:

.. ref-code-block:: jnc

	class MyDisposableClass
	{
		void close(); // a natural name for a resource-releasing method
	    alias dispose = close;
	}
