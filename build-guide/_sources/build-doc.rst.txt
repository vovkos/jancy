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

Building Documentation
======================

Jancy contains four documentation packages:

* Build guide
* Language manual
* Standard library reference
* API reference

.. expand-macro:: build-doc-intro ./build/jancy

Build Guide
-----------

.. expand-macro:: build-doc-build-guide ./build/jancy

Language Manual
---------------

A book on specific features of Jancy language.

Documentation sources are located at: ``./doc/language``

Build steps:

.. code-block:: bash

	cd ./build/jancy/doc/language
	./build-html
	./build-pdf

Standard Library Reference
--------------------------

A reference on Jancy standard library.

Documentation sources are located at: ``./doc/stdlib``

Before building stdlib documentation you need to build Jancy first -- ``jancy`` compiler is required to analyze stdlib sources and extract documentation comments.

``doxyrest`` is also needed to convert Doxygen XML database to Re-Structured Text (which will be further passed to ``sphinx-build``).

Once ``jancy`` and ``doxyrest`` are ready, you can run the scripts (replace ``Debug`` with ``Release`` if you have built ``jancy`` and ``doxyrest`` under ``Release`` configuration).

Build steps:

.. code-block:: bash

	cd ./build/jancy/doc/language
	./build-xml Debug # replace with Release if jancy was built for Release
	./build-rst Debug # replace with Release if doxyrest was built for Release
	./build-html
	./build-pdf

API Reference
-------------

A reference on Jancy API for C/C++. This is the documentation for those who want to write static or dynamic extension libraries or plan to embed Jancy into their C/C++ applications as a scripting engine.

.. expand-macro:: build-doc-doxygen ./build/jancy api
