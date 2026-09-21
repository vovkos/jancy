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

Exception Handling
==================

Jancy exceptions handling model applies a layer of syntactic sugar over good old C-style error code checking. As a result, it is extremely transparent and easy to support from the host C/C++ application.

A function marked by the ``errorcode`` modifier will have its return value interpreted as an error code. Intuitive defaults are assumed:

* ``false`` for bools;
* **any negative value** for signed integers;
* ``-1`` for unsigned integers;
* ``null`` for pointers;
* everything else is cast to ``bool``.

.. code-block:: jnc

	bool errorcode foo(int a) {
		printf("foo(%d)\n", a);
		return a > 0;
	}

An enum follows its base type -- an enum over a signed integer (the default) treats any negative enumerator as an error, while an enum over an unsigned one only recognizes ``-1``. This makes it natural to declare a set of distinct error codes:

.. code-block:: jnc

	enum LoadResult {
		PassphraseError = -2,
		Error           = -1,
		Success         =  0,
	}

	LoadResult errorcode loadPrivateKey(string_t fileName, string_t passphrase);

If return values match, the error code is automatically propagated:

.. code-block:: jnc

	int errorcode foo(int a);

	int errorcode bar(int a) {
		// ...

		baz(a);

		// ...
	}

The specific error code is preserved while it propagates -- as long as it still reads as an error in the receiving function. Propagating between signed integers keeps the value (a ``-2`` stays a ``-2``), while propagating into a bool, a pointer or an unsigned integer -- or narrowing it so far that the sign is lost (e.g. ``-256`` into an ``int8_t``) -- falls back to the canonical error code of the target type.

The same applies to the value yielded by the ``try`` operator, so a caller can tell one error code from another:

.. code-block:: jnc

	LoadResult result = try loadPrivateKey(fileName, passphrase);
	if (result == LoadResult.PassphraseError) {
		// ask the user for a passphrase and try again
	}

The ``try`` operator shields an expression from **throwing**:

.. code-block:: jnc

	int result = try baz(-5);

The ``try`` block shields a parent scope from **throwing** even if this parent scope has no ``catch``:

.. code-block:: jnc

	void foo() {
		// ...

		try {
			baz(20);
			baz(-1);
			baz(21); // never get here
		}

		// ...
	}

``catch`` and ``finally`` can be within any scope:

.. code-block:: jnc

	int errorcode bar(int a) {
		// ...

	catch:
		printf("bar.catch\n");
		return -5;

	finally:
		printf("bar.finally\n");
	}

When calling a function, the developer can use either an error code check or exception semantics depending on what's more appropriate or convenient in each particular case.

.. code-block:: jnc

	int main() {
		// ...

		int result = try bar();
		if (result < 0) {
			// handle error
		}
	}
