Introduction
============

Abstract
~~~~~~~~

Jancy is a scripting programming language with LLVM back-end. Jancy offers a lot of convenient features for low-level IO (input-output) and UI (user-interface) programming which are not found in mainstream languages (and sometimes, nowhere else). This includes safe pointer arithmetics, high level of source-level and ABI compatibility with C/C++, reactive programming, built-in generator of incremental lexers/scanners and a lot more.

Motivation
~~~~~~~~~~

Why create yet another programming language? Like, there isn't enough of them already?

I have asked myself these questions hundreds of times, over and over again. I can name dozens of possible arguments against creating a new programming language. I understand all the difficulties a language creator is doomed to face before he can gather any meaningful number of users of the new language. And I still believe that creation of Jancy was justified. The truth is, Jancy was not created just to fix the infamous fatal flaw of other languages (aka \emph{they didn't write it}). Of course, the passion to invent was a significant driving force, but there was \emph{practival} reasoning besides that. 

During development of a product called IO Ninja (a universal all-in-one low-level IO debugger) we were looking for a scripting language with the support for safe pointers and safe pointer arithmetics. After not finding one, we basically had a simple choice: either we settle for some existing scripting language without safe pointer arithmetics (say, embedded Python) or create a new scripting language -- tailor-suited for this task. Create it both for ourselves, and for other developers who could be in need for a scripting language really good at handling binary data. Needless to say, we had chosen the later option -- otherwise, you would not be reading this manual.

Of course, besides featuring safe pointers and safe pointer arithmetics, Jancy offers a long list of other useful but rarely found facilities. So let's outline the distinguishing features of Jancy below. And for those of you wondering \emph{what's in a name}, Jancy is an acronym: [in-between] Java-and-C.

Design Principles
~~~~~~~~~~~~~~~~~

* Object-oriented scripting language for IO and UI programming with C-family syntax
* ABI (application-binary-interface) compatibility with C
* Automatic memory management via accurate GC (garbage collection)
* LLVM (Low Level Virtual Machine) as a back-end

Key Features
~~~~~~~~~~~~

* Safe pointers and pointer arithmetic
* Unprecedented for scripting languages source-level compatibility with C
* Built-in Reactive Programming support
* Built-in regexp-based generator of incremental lexers/scanners

Other Notable Features
~~~~~~~~~~~~~~~~~~~~~~

* Exception-style syntax over error code checks
* Properties -- the most comprehensive implementation thereof
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

