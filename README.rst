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

Jancy
=====
.. image:: https://travis-ci.org/vovkos/jancy.svg?branch=master
	:target: https://travis-ci.org/vovkos/jancy

Abstract
--------

Jancy is a **scripting programming language** with the **LLVM** back-end. Jancy offers a lot of convenient features for low-level IO (input-output) and UI (user-interface) programming which are not found in mainstream languages (and sometimes, nowhere else). This includes **safe pointer** arithmetics, high level of source-level and ABI **compatibility with C**, **reactive programming**, built-in generator of **incremental lexers/scanners** and a lot more.

Design Principles
-----------------

* Object-oriented scripting language for IO and UI programming with C-family syntax
* ABI (application-binary-interface) compatibility with C
* Automatic memory management via accurate GC (garbage collection)
* LLVM (Low Level Virtual Machine) as a back-end

Key Features
------------

* Safe pointers and pointer arithmetic
* Unprecedented for scripting languages source-level compatibility with C
* Built-in Reactive Programming support
* Built-in regexp-based generator of incremental lexers/scanners
* Deterministic resource release
* Error handling model which allows both throw-catch semantics and error code checks -- with the same function!

Other Notable Features
----------------------

* Properties (the most comprehensive implementation thereof)
* Multicasts and events (including weak events, which do not require to unsubscribe)
* Multiple inheritance
* Const-correctness
* Thread local storage
* Weak pointers (do not retain objects)
* Partial application for functions and properties
* Scheduled function pointers
* Bitflag enums
* Perl-style formatting
* Hexadimal, binary and multi-line literals

Documentation
-------------

* `Jancy Language Manual <http://docs.tibbo.com/jancy/language>`_
* `Jancy Standard Library Reference <http://docs.tibbo.com/jancy/stdlib>`_
* `Jancy C API Reference <http://docs.tibbo.com/jancy/api>`_
* `Jancy Compiler Overivew <http://docs.tibbo.com/jancy/compiler>`_
* `Jancy Grammar Reference <http://docs.tibbo.com/jancy/grammar>`_
* `Jancy Build Guide <http://docs.tibbo.com/jancy/build-guide>`_
