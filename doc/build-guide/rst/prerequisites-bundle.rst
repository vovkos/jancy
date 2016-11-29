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

Bundled Frameworks
==================

Items in this category are bundled in the recommended ``jancy_b`` bundle package. Therefore, if you build Jancy from ``jancy_b`` package, you can safely skip the current section -- all the following dependencies are already included and will be built automatically during the build process.

However, if you use the standalone ``jancy`` package, you will need to download and build these dependencies first.

* AXL

	Jancy and Graco use AXL as a general purpose C++ support library.

	AXL is a lightweight C++ library featuring:

	- Java naming convention;
	- non-zero-terminated UTF-8 string slices as a default string-passing model;
	- TLS-based error-handling system;
	- unified reference-counting model;
	- wrappers for many popular libraries;
	- and more...

	Sources of AXL are available at: http://tibbo.com/downloads/archive/axl

* Graco

	Jancy uses grammar compiler Graco for generating the parser of its front-end.

	Graco is a EBNF-based generator of table-driven top-down parsers of LL(k) grammars featuring:

	- predictable & configurable conflict resolution mechanism;
	- retargetable back-end (via Lua string templates);
	- ANYTOKEN support;
	- external tokenization loop;
	- convenient syntax for passing and returning rule arguments;
	- and more...

	Sources of Graco are available at: http://tibbo.com/downloads/archive/graco
