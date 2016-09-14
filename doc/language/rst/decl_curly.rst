Curly Initializers
==================

Jancy supports a convenient method of assigning aggregate values with curly initializers:

Classic C-style curly-intializers:

.. code-block:: none

	int a [] = { 1, 2, 3 };

It's OK to skip elements leaving them zero-initialized:

.. code-block:: none

	int b [10] = { ,, 3, 4,,, 7 };

You can use both index- and name-based addressing:

.. code-block:: none

	struct Point
	{
	    int m_x;
	    int m_y;
	    int m_z;
	}

	Point point = { 10, m_z = 30 };

You can also use curly-initializers in assignment operator after declaration:

.. code-block:: none

	point = { , 200, 300 };

...or in a new operator:

.. code-block:: none

	Point* point2 = new Point { m_y = 2000, m_z = 3000 };
