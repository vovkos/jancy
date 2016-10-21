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

Simple Property Declaration
===========================

Jancy supports what I believe to be the most natural syntax for declaring properties:

.. code-block:: none

	int property g_simpleProp;

This syntax is ideal for declaring interfaces or when the developer prefers to follow the C++-style of placing definitions outside of a class:

.. code-block:: none

	int g_simpleProp.get ()
	{
	    return rand () % 3;
	}

	g_simpleProp.set (int x)
	{
	    // set property value
	}

Const properties can use a simple declaration syntax:

.. code-block:: none

	int const property g_simpleReadOnlyProp;

	int g_simpleReadOnlyProp.get ()
	{
	    return rand () % 3;
	}

For obvious reasons, this simple syntax is only possible if a property has no overloaded setters, in which case you should use the second method: full property declaration.
