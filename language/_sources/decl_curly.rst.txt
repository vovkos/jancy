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

Curly Initializers
==================

Jancy supports a convenient method of assigning aggregate values with curly initializers:

Classic C-style curly-intializers:

.. code-block:: jnc

	int a[] = { 1, 2, 3 }

It's OK to skip elements leaving them zero-initialized:

.. code-block:: jnc

	int b[10] = { ,, 3, 4,,, 7 }

You can use both index- and name-based addressing:

.. code-block:: jnc

	struct Point {
		int m_x;
		int m_y;
		int m_z;
	}

	Point point = { 10, m_z = 30 };

You can also use curly-initializers in assignment operator after declaration:

.. code-block:: jnc

	point = { , 200, 300 }

...or in a new operator:

.. code-block:: jnc

	Point* point2 = new Point { m_y = 2000, m_z = 3000 }

Have you noticed, there are no ';' terminators after the curly-initializers? In Jancy you can omit those.

Also, when initializing char arrays, you can mix string literals in:

.. code-block:: jnc

	char buffer[] = {
		10,
		20,
		"null-terminated",
		30,
		r"\non\null\terminated\",
		40,
		0x"ab cd ef"
	}
