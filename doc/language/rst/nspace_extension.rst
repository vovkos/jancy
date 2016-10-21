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

Extension Namespaces
====================

Jancy offers a way to extend the functionality of existing classes with extension namespaces. An extension namespace declares additional methods which have access to all the members of the class that they extend. There are certain limitations imposed on the extension methods. These ensure that if your code runs without extension namespaces, then it runs exactly the same with the introduction of any extension namespace(s):

.. code-block:: none

	class C1
	{
	    protected int m_x;

	    construct (int x)
	    {
	        printf ("C1.construct (%d)\n", x);
	        m_x = x;
	    }

	    foo ()
	    {
	        printf ("C1.foo () { m_x = %d }\n", m_x);
	    }
	}

	extension ExtC1: C1
	{
	    bar ()
	    {
	        // extension method has access to protected data
	        printf ("C1 (extend).bar () { m_x = %d }\n", m_x);
	    }

	    static baz ()
	    {
	        printf ("C1 (extend).baz ()\n");
	    }

	    // constructors cannot be part of extension namespace
	    construct (double x); // error

	    // operator methods cannot be part of extension namespace
	    int operator += (int x); // error

	    // virtual methods cannot be part of extension namespace
	    virtual baz (); // error
	}

	// entry point

	int main ()
	{
	    C1 c construct (100);
	    c.foo ();
	    c.bar ();  // bar () is extension method
	    C1.baz (); // baz () is static extension method
	    return 0;
	}
