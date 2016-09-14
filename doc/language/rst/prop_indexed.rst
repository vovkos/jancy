Indexed Properties
==================

Jancy also supports **indexed** properties, which are properties with array semantics. Accessors for such properties accept additional index arguments, but unlike a real array index, a property index doesn't have to be of the integer type, nor does it mean **index** exclusively -- it is up to the developer how to use it.

Simple indexed property declaration syntax:

.. code-block:: none

	int g_x [2];

	int indexed property g_simpleProp (unsigned i);

	// here the index argument is really used as the array index

	int g_simpleProp.get (unsigned i)
	{
	    return g_x [i];
	}

	g_simpleProp.set (
	    unsigned i,
	    int x
	    )
	{
	    g_x [i] = x;
	}

A similar property declared using full syntax:

.. code-block:: none

	property g_prop
	{
	    int m_x [2] [2];

	    // more than one index argument could be used

	    int get (
	        unsigned i,
	        unsigned j
	        )
	    {
	        return m_x [i] [j];
	    }

	    set (
	        unsigned i,
	        unsigned j,
	        int x
	        )
	    {
	        m_x [i] [j] = x;
	    }

	    // setters of indexed property can be overloaded

	    set (
	        unsigned i,
	        unsigned j,
	        double x
	        )
	    {
	        m_x [i] [j] = (int) x;
	    }
	}

Accessing indexed properties looks like accessing arrays

.. code-block:: none

	int indexed property g_prop (
	    unsigned i,
	    unsigned j
	    );

	foo ()
	{
	    int value = g_prop [10] [20];

	    // ...

	    g_prop [30] [40] = 100;

	    // ...
	}