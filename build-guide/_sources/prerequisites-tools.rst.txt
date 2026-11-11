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

Tools
=====

.. expand-macro:: tools-intro Jancy

Required Tools
--------------

These tools are **required** for building the Jancy compiler:

* CMake 3.3 or above

	.. expand-macro:: cmake-common-info Jancy

* Ragel

	Jancy uses Ragel as a lexer/scanner generator for the tokenization stage of its front-end.

	.. expand-macro:: ragel-common-info

* Perl

	Jancy uses Perl to convert Jancy source files containing definitions of standard types and functions into C++ code snippets, which are then included in Jancy compiler C++ source files.

	.. expand-macro:: perl-common-info

* 7-Zip

	Jancy uses 7-Zip file archiver to package dynamic extensions.

	.. expand-macro:: 7zip-common-info

.. _optional-tools:

Optional Tools
--------------

These tools are **optional** and only needed if you plan to build Jancy documentation:

* Python

	.. expand-macro:: python-sphinx-common-info Jancy

* Doxygen

	.. expand-macro:: doxygen-common-info Jancy

* Doxyrest Jancy jancy

	Jancy documentation uses Doxyrest as a middle-end in ``doxygen-doxyrest-sphinx`` or ``jancy-doxyrest-sphinx`` pipelines.

	.. expand-macro:: doxyrest-common-info

* Sphinx

	.. expand-macro:: sphinx-common-info Jancy
