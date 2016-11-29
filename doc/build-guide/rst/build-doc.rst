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

Building documentation
======================

Jancy contains four documentation packages:

* Build guide

	This is what you are reading right now.

	Located at: ``./doc/build-guide``

* Language manual

	A book on specific features of Jancy language.

	Located at: ``./doc/language``

* Stdlib reference

	A reference on Jancy standard library.

	Located at: ``./doc/stdlib``

* API reference

	A reference on Jancy API for C/C++. This is documentation for those who want to write static or dynamic extension libraries or plan to embed Jancy into their C/C++ applications as a scripting engine.

	Located at: ``./doc/api``

If you have all the necessary :ref:`prerequisites <optional-tools>` for building documentation, CMake should have created all the necessary shell scripts under: ``./build/doc/<doc-package>``

Let's say, you want to build HTML version of Jancy Standard Library Reference.

Before building stdlib documentation you need to build Jancy first -- Jancy compiler is requred to analyze stdlib sources and extract doxygen-style documentation comments. Once it's done, you can run the scripts:

.. rubric:: Windows

::

	cd ./build/doc/stdlib
	./build-html.bat Debug

Or, if you have built a ``Release`` configuration of Jancy compiler::

	./build-html Release

.. rubric:: Unix

::

	cd ./build/doc/stdlib
	./build-html


The resulting HTML pages will be placed at ``./build/doc/stdlib/html``.
