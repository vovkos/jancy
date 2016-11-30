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

If you have required :ref:`prerequisites <optional-tools>` for building documentation, CMake should have created all the necessary shell scripts under: ``./build/doc/<doc-package>``

``sphinx-build`` is always needed; ``pdflatex`` is also needed to build PDF.

Resulting HTML pages will be placed at: ``./build/doc/<doc-package>/html``

Resulting PDF will be placed at: ``./build/doc/<doc-package>/pdf``

Build Guide
-----------

This is what you are reading right now.

Located at: ``./doc/build-guide``

Build steps:

.. code-block:: bash

	cd ./build/doc/build-guide
	./build-html
	./build-pdf

Language Manual
---------------

A book on specific features of Jancy language.

Located at: ``./doc/language``

Build steps:

.. code-block:: bash

	cd ./build/doc/language
	./build-html
	./build-pdf

Standard Library Reference
--------------------------

A reference on Jancy standard library.

Located at: ``./doc/stdlib``

Before building stdlib documentation you need to build Jancy first -- ``jancy`` compiler is requred to analyze stdlib sources and extract documentation comments.

``doxyrest`` is also needed to convert Doxygen XML database to Re-Structured Text (which will be further passed to ``sphinx-build``).

Once ``jancy`` and ``doxyrest`` are ready, you can run the scripts (replace ``Debug`` with ``Release`` if you have built ``jancy`` and ``doxyrest`` under ``Release`` configuration).

Build steps:

.. code-block:: bash

	cd ./build/doc/language
	./build-xml Debug # replace with Release if jancy was built for Release
	./build-rst Debug # replace with Release if doxyrest was built for Release
	./build-html
	./build-pdf

API Reference
-------------

A reference on Jancy API for C/C++. This is the documentation for those who want to write static or dynamic extension libraries or plan to embed Jancy into their C/C++ applications as a scripting engine.

Located at: ``./doc/api``

``doxygen`` is additionally needed to analyze API headers and extract documentation comments.

``doxyrest`` is also needed to convert Doxygen XML database to Re-Structured Text (which will be further passed to ``sphinx-build``).

Once ``doxygen`` and ``doxyrest`` are ready, you can run the scripts (replace ``Debug`` with ``Release`` if you have built ``doxyrest`` under ``Release`` configuration).

Build steps:

.. code-block:: bash

	cd ./build/doc/api
	./build-xml
	./build-rst Debug # replace with Release if doxyrest was built for Release
	./build-html
	./build-pdf
