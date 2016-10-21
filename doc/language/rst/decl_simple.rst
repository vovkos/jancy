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

Simple Declarations
===================

Syntax of declarations in Jancy is common for all the C-family languages and can be expressed with the following formula: **specifier(s)-declarator(s)**.

.. code-block:: none

	int a;

In example above ``int`` is a type specifier and ``a`` is a declarator. This declaration creates a new integer variable (or field) named ``a`` -- just like you would expect in C-world. Unlike C/C++ language though, any uninitialized variable in Jancy is **zero-initialized**, so ``a`` will be holding **zero**.

Declaring **functions** in Jancy is also similar to any C-family language:

.. code-block:: none

	void foo (
	    int a,
	    double b
	    )
	{
	    // ...
	}

This defines a function which return no value and takes two arguments of types ``int`` and ``double`` -- again, nothing unexpected here.

It's important to say a couple of words on **arrays**, however. In Java, C\#, D and other modern languages arrays are **dynamically-sized**; in C/C++ compiler-generated arrays are **fixed-sized** while dynamically-sized arrays are implemented as classes. Since being able to copy-paste C/C++ declarations of network protocol headers was crucial, Jancy adopts C/C++ model:

.. code-block:: none

	int a [10] [20];

This declares a two-dimensional array of integers (10 rows and 20 columns) named ``a``. Now let's proceed to something more complex.
