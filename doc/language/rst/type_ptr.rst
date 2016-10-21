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

Pointer Types
=============

Unsafe as they are, pointers are not something we can live without. Even languages without pointers (like Basic or Java) have pointers to classes and interfaces.

Pointers has always been considered an unsafe tool that could easily cause a program to crash or (worse!) silently corrupt user data.

Even if we limit the number of pointer kinds and pointer operations available to developers, we still won't be able to perform the complete analysis of pointer-related correctness at compile time.

For the purpose of the discussion that follows let's define **pointer safety**.

We will call a pointer **safe** if it's impossible to either crash a program or corrupt user data by accessing this pointer. This means that any invalid pointer access will be caught and handled by the language runtime.

.. toctree::
	:titlesonly:

	type_ptr_data.rst
	type_ptr_class.rst
	type_ptr_const.rst
	type_ptr_function.rst
	type_ptr_property.rst
