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

Property Pointers
=================

Property pointers are yet another unique feature of Jancy.

Properties are found in many modern languages. What commonly lacks is a developed syntax and semantics of pointer declarations and operators.

Property pointers resemble and are closely related to function pointers. Dealing with property pointers requires a more careful application of address **\&** and indirection **\*** operators. This is due to the possibility of implicit invocation of property accessors and the ambiguity induced by such invocation, which can be automatically resolved with function pointers and not with property pointers.

Like the function pointers, property pointers can be normal (fat) or ``thin``. ``thin`` property pointers hold a single pointer to the property accessor table.

.. code-block:: jnc

	int autoget property g_prop;

	g_prop.set(int x) {
		// ...
	}

	void foo() {
		int property thin* p = &g_prop;
		*p = 10;
	}

Like with the function pointers, the argument conversion is automated (compiler generates thunks if needed).

.. code-block:: jnc

	int autoget property g_prop;

	g_prop.set(int x) {
		// ...
	}

	void foo() {
		typedef double property FpProp;

		// explicit cast is required to generate a thunk
		FpProp thin* p = (FpProp thin*)&g_prop;

		*p = 2.71;
	}

Fat property pointers support partial application by capturing arguments in the closure.

.. code-block:: jnc

	class C1 {
		int autoget property m_prop;

		m_prop.set(int x) {
			// ...
		}
	}

	void foo() {
		C1 c;
		int property* p = c.m_prop;
		*p = 100;
	}

It is also possible to capture index arguments in the closure, thus reducing dimensions of indexed properties or completely de-indexing them. Skipping indexes is OK, too.

.. code-block:: jnc

	property g_prop {
		int get(
			unsigned i,
			unsigned j
		) {
			// ...
		}

		set(
			unsigned i,
			unsigned j,
			int x
		) {
			// ...
		}
	}

	void foo() {
		int indexed property* p (unsigned) = g_prop[][20];
		*p[10] = 100; // => g_prop[10][20] = 100;
	}

Like function pointers, property pointers can be ``weak``, meaning that they do not retain selected objects in the closure from being collected by the garbage collector.

.. code-block:: jnc

	class C1 {
		int autoget property m_prop;

		// ...
	}

	C1.m_foo.set(int x) {
		// ...
	}

	void foo() {
		C1* c = new C1;

		int property weak* w = &c.m_prop;

		// uncomment the next line and C1 will get collected next gc run
		// c = null;

		std.collectGarbage();

		int property* p = w;
		if (p) {
			// object survived GC run, access it
			*p = 100;
		}

		return 0;
	}
