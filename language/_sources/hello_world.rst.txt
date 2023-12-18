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

Hello World
-----------

Tradition dictates to start any language book with a **"Hello World"** program, so I'm not going to re-invent the wheel in this paricular case:

.. code-block:: jnc

	int main ()
	{
	    printf ("hello world!\n");
	    return 0;
	}

No explanation is needed, I suppose? Keeping Jancy language syntax as close to C/C++/Java as possible was imperative, and I believe a very pleasant level of source-level compatibility was eventually achieved. Any experienced C, C++ or Java programmer should be able to read and understand most of Jancy code from the get-go, without any special training. Moreover, often times it is possible to simply copy-paste C/C++/Java code snippets into Jancy source files and compile them with little to no modifications.
