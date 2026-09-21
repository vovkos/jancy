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

Deterministic Resource Release
==============================

Jancy provides a convenient facility for ensuring the deterministic resource release in an inherently non-deterministic GC-world. This is achieved with ``disposable`` storage specifier.

Classes in Jancy can have destructors. Therefore, all the necessary resource release operations (e.g. closing files, dropping connections, releasing locks etc) could be placed in destructors. The problem is, destructors in garbage-collected world are not called deterministically. First the Garbage Collector must decide it needs to free unreachable objects; then the mark stage must complete. Only then the destructor of an about-to-be-freed object will be called. Keeping resources acquired for that long is simply not acceptable.

To remedy this problem, Jancy features duck-typed pattern **Disposable**. All the variables (of ``class``, ``struct`` or ``union`` types) declared using ``disposable`` storage will have their ``dispose`` method called upon exiting the scope.

This method will be called no matter which exit route is taken, be it normal control flow, ``return``, ``break``, ``throw``, other exception(s) etc.

.. rubric:: Example:
.. ref-code-block:: jnc

	void foo() {
		// ...

		disposable io.File file;
		file.open();

		// work with file...
	} // <-- file.close() is guaranteed to be called upon exiting the scope

When program runs out of scope where a variable of disposable class was declared in, Jancy compiler will insert a call to ``dispose`` method (which is usually *aliased* to an actual release methods such as ``close``). This method will be called no matter how the program runs out of scope, be it a normal control flow, ``return``,  ``break``, ``continue`` or an exception.

You can easily write disposable classes yourself. All you have to do is to provide a ``dispose`` method (or a ``dispose`` alias):

.. ref-code-block:: jnc

	class MyDisposableClass {
		//...
		void dispose() {
			printf("Releasing resources...");
		}
	}

	class MyDbConnection {
		//...
		void disconnect();
		alias dispose = disconnect;
	}

	void foo() {
		// ...

		disposable MyDisposableClass c;
		disposable MyDbConnection db;

		// work with c & db...

		throw; // <-- c.dispose() and db.disconnect() will get called
	}
