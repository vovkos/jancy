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

Class Pointers
==============

Pointer arithmetic is not applicable to class pointers, therefore, class pointer validity can be ensured by performing a null-check on access and a scope level check on assignment. Scope level information could be stored in class header instead of class pointer, so class pointer does not need to be **fat** to be safe.

Jancy provides built-in support for a special kind of class pointers: **weak**. These pointers do not affect the lifetime of an object. Obviously, weak class pointers cannot be used to access an object they point to and can only be cast to strong pointers. If this cast operation returns non-null value, the result can be used to access the object normally; otherwise, the object has already been destroyed.

.. code-block:: jnc

	class C1
	{
	    //...
	}

	foo ()
	{
	    C1* c = new C1;
	    C1 weak* w = c;

	    // if we lose a strong pointer before GC run, the object will be collected

	    jnc.runGc ();

	    c = w; // try to restore strong pointer

	    if (c)
	    {
	        // the object is still alive
	    }
	}
