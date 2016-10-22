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

Autoget Properties
==================

In most cases a property getter is supposed to return a variable value or field, and all of the property logic is contained in the property setter. Jancy takes care of this common case by providing autoget properties. Such properties do not require a getter implementation: the compiler will access the data variable/field directly if possible, or otherwise generate a getter to access it.

Simple syntax for declaring autoget property:

.. code-block:: jnc

	int autoget property g_simpleProp;

	g_simpleProp.set (int x)
	{
	    m_value = x; // name of compiler-generated field is 'm_value'
	}

The same property declared using full syntax:

.. code-block:: jnc

	property g_prop
	{
	    int autoget m_x; // 'autoget' field implicitly makes property 'autoget'

	    set (int x)
	    {
	        m_x = x;
	    }

	    // setters of autoget property can be overloaded

	    set (double x)
	    {
	        m_x = (int) x;
	    }
	}

Autoget and indexed property modifiers are mutually exclusive.
