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

Const-correctness
=================

As a language with pointers, Jancy fully implements the paradigm of const-correctness. The core idea behind const-correcness is to specifically mark pointers that cannot be used to modify the target object.

Admittedly, const-correctness generally makes it harder to design interfaces and APIs in general as it becomes yet another item for the developer to worry about. At the same time const-correctness greatly improves both the overall type-safety of the language and its ability to self-document.

As in C++, use the ``const`` modifier to define a const-pointer.

.. code-block:: jnc

	struct Point {
		int m_x;
		int m_y;
	}

	void transpose(
		Point* dst,
		Point const* src // we can be sure 'src' is not going to change
	) {
		int x = src.m_x; // so it works even when dst and src point to the same location
		dst.m_x = src.m_y;
		dst.m_y = x;

		// src.m_x = 0; // error: cannot store into const location
	}

All non-static methods implicitly accept an extra ``this`` argument, so it is necessary to be able to specify whether ``this`` is ``const`` or not -- if yes, then such a method is called a const method.

Certain fields can be modified even from const methods (for example, various kinds of cache fields) -- these are mutable fields.

The syntax for declaring const methods and mutable fields has also been borrowed from C++:

.. code-block:: jnc

	void class C1 {
		int m_field;
		mutable int m_mutableField;

		void foo() const {
			// ...
		}

		void bar(int x) {
			// ...
		}
	}

	void baz(C1 const* p) {
		p.foo();               // ok, const method
		p.m_mutableField = 100; // ok, mutable field

		p.m_field = 200; // error: cannot store to const location
		p.bar(200);     // error: cannot convert 'C1 const*' to 'C1*'
	}
