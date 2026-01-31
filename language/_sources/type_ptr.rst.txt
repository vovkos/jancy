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

Unsafe as they are, pointers are not something we can live without. Even languages without pointers (like **Basic or Java**) **have pointers** -- to classes and interfaces!

Pointers has always been considered an **unsafe tool** which could easily cause a program to **crash** or (worse!) silently **corrupt user data**. The worst thing is, it's **impossible** to perform the complete **analysis** of pointer-related correctness **at compile time** without imposing crippling limitations on the whole ecosystem of pointer operations available to developers. Bottom line is, **invalid pointer** accesses can and **will happen at runtime**.

So what is **safety**, then?

We will call a pointer **"safe"** if it's impossible to either crash a process or corrupt user data by accessing this pointer. This means that any **invalid pointer** access **must be caught** and handled by the language runtime.

.. toctree::
	:titlesonly:

	type_ptr_data.rst
	type_ptr_class.rst
	type_ptr_const.rst
	type_ptr_function.rst
	type_ptr_property.rst
