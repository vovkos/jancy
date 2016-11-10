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

Introduction
============

Jancy API is the public interface for C++ and pure C languages aimed at those developers who want to use Jancy engine. That includes embedding Jancy into applications, using it as an application-customizing scripting language and writing standalone extension libraries for Jancy in C or C++ language.

Jancy API could be logically divided into the the following large parts:

* Compile subsystem
* Runtime subsystem
* Extension subsystem

One of the main goals in design of Jancy was making it easy to extend. That applies both to creating extension libraries for Jancy AND to invoking stadnard/system libraries *directly*, without creating any extra so-called *bindings* required in other scripting languages.

