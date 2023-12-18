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

Full Property Declaration
=========================

A full property declaration looks a lot like a declaration for a class. It implicitly opens a namespace and allows for overloaded setters, member fields, helper methods, constructors/destructors etc.

.. code-block:: jnc

	property g_prop
	{
	    int m_x = 5; // member field with in-place initializer

	    int get ()
	    {
	        return m_x;
	    }

	    set (int x)
	    {
	        m_x = x;
	        update ();
	    }

	    set (double x); // overloaded setter
	    update (); // helper method
	}

A body of a method can be placed right away (Java-style *inline* body definition) or outside -- maybe even in another file (C++-style *out-of-line* body definition).
