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

.. image:: https://github.com/vovkos/jancy/actions/workflows/ci.yml/badge.svg
	:target: https://github.com/vovkos/jancy/actions/workflows/ci.yml
.. image:: https://ci.appveyor.com/api/projects/status/01gq23xd13twr8l5?svg=true
	:target: https://ci.appveyor.com/project/vovkos/jancy
.. image:: https://img.shields.io/badge/donate-@jancy.org-blue.svg
	:align: right
	:target: http://jancy.org/donate.html?donate=jancy

Abstract
--------

.. raw:: html

	<a href="http://jancy.org"><img src="doc/mascot/jancy-215x350.png" alt="Jancy" align="right"></a>

Jancy is *the first and only* scripting language with **safe pointer arithmetics**, high level of ABI and source **compatibility with C**, support for spreadsheet-like **reactive programming**, built-in generator of **incremental lexers/scanners**, **dynamic structures** with array fields of non-constant sizes, **dual error handling model** which allows you to choose between error-code checks and throw semantics at each *call-site*, and a lot of other unique and really useful features.

Design Principles
-----------------

* Statically typed C-family scripting language for IO and UI domains

	Python is *the* scripting language of hackers. I hope Jancy will one day become the scripting language of those hackers who prefer to *stay closer to C*.

* High level of ABI and source compatibility with C

	Calling from Jancy to native code and vice versa is as *easy and efficient* as it gets. So is developing Jancy libraries in C/C++ and Jancy bindings to popular libraries. So is porting publicly available algorithms from C to Jancy -- *copy-paste* often suffices!

* Automatic memory management via accurate GC

	Losing manual memory management (together with the vast class of bugs and leaks associated with it) in favor of the GC employment has its price, but for scripting languages, it's 100% worth it.

* LLVM as a back-end

	This was a no-brainer from the very beginning. I started with LLVM 3.1 a few years ago; at the present moment Jancy builds and runs with any LLVM version from 3.4.2 all the way up to the latest and greatest LLVM 8.0.0

Key Features
------------

Safe Pointers and Pointer Arithmetic
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use pointer arithmetic -- the most elegant and the most efficient way of parsing and generating binary data -- and do so without worrying about buffer overruns and other pointer-related issues!

.. code:: cpp

	IpHdr const* ipHdr = (IpHdr const*)p;
	p += ipHdr.m_headerLength * 4;

	switch (ipHdr.m_protocol) {
	case Proto.Icmp:
		IcmpHdr const* icmpHdr = (IcmpHdr const*)p;
		switch (icmpHdr.m_type) {
		case IcmpType.EchoReply:
			...
		}
		...
	}

If bounds-checks on a pointer access fail, Jancy runtime will throw an exception which you can handle the way you like.

You can also safely pass buffers from C/C++ to Jancy without creating a copy on the GC-heap:

.. code:: cpp

	void thisIsCpp(
		jnc::Runtime* runtime,
		jnc::Function* function
		) {
		char buffer[] = "I'm on stack but still safe!";

		JNC_BEGIN_CALL_SITE(runtime)
			// create a call-site-local foreign data pointer
			jnc::DataPtr ptr = jnc::createForeignBufferPtr(buffer, sizeof(buffer), true);
			jnc::callFunction(function, ptr);
		JNC_END_CALL_SITE() // here ptr is invalidated and Jancy can't access it anymore
	}

Spreadsheet-like Reactive Programming
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Write auto-evaluating *formulas* just like you do in Excel -- and stay in full control of where and when to use this spreadsheet-likeness:

.. code:: cpp

	reactor m_uiReactor {
		m_title = $"Target address: $(m_addressCombo.m_editText)";
		m_localAddressProp.m_isEnabled = m_useLocalAddressProp.m_isChecked;
		m_isTransmitEnabled = m_state == State.Connected;
		...
	}

	m_uiReactor.start();
	// now UI events are handled inside the reactor...

	m_uiReactor.stop();
	// ...and not anymore

This, together with the developed infrastructure of *properties* and *events*, is perfect for UI programming!

Scheduled Function Pointers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Schedulers* allow you to elegantly place the execution of your *callback* (completion routine, event handler, etc) in the correct environment -- for example, into the context of a specific thread:

.. code:: cpp

	class WorkerThread: jnc.Scheduler {
		override schedule(function* f()) {
			// enqueue f and signal worker thread event
		}
		...
	}

Apply a binary operator ``@`` (reads *"at"*) to create a *scheduled* pointer to your callback:

.. code:: cpp

	void onComplete(bool status) {
		// we are in the worker thread
	}

	WorkerThread workerThread;
	startTransaction(onComplete @ workerThread);

When the transaction completes and completion routine is finally called, ``onComplete`` is guaranteed to be executed in the context of the assigned ``workerThread``.

Async-Await (with A Cherry On Top)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The async-await paradigm is becoming increasingly popular during recent years -- and righfully so. In most cases, it absolutely is **the right way** of doing asynchronous programming. As a language targeting the IO domain, Jancy fully supports async-await:

.. code:: cpp

	async void transact(char const* address) {
		await connect(address);
		await modify();
		await disconnect();

	catch:
		handleError(std.getLastError());
	}

	jnc.Promise* promise = transact();
	promise.blockingWait();

A cherry on top is that in Jancy you can easily control the *execution environment* of your ``async`` procedure with *schedulers* -- for example, run it in context of a specific thread:

.. code:: cpp

	// transact() will run in the worker thread
	jnc.Promise* promise = (transact @ m_workerThread)("my-service");

You can even switch contexts during the execution of your ``async`` procedure:

.. code:: cpp

	async void foo() {
		await thisPromise.asyncSetScheduler(m_workerThread);
		// we are in the worker thread

		await thisPromise.asyncSetScheduler(m_mainUiThread);
		// we are in the main UI thread
	}

Incremental Regex-based Switches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create *efficient* regex-based switches for tokenizing string streams:

.. code:: cpp

	jnc.RegexState state;
	reswitch (state, p, length) {
	case "foo":
		...
		break;

	case r"bar(\d+)":
		print($"bar id: $(state.m_subMatchArray[0].m_text)\n");
		break;

	case r"\s+":
		// ignore whitespace
		break;

	...
	}

This statement will compile into a table-driven DFA which can parse the input string in ``O(length)`` -- you don't get any faster than that.

But there's more -- the resulting DFA recognizer is *incremental*, which means you can feed it the data chunk-by-chunk when it becomes available (e.g. once received over the network).

Dynamic Structs
~~~~~~~~~~~~~~~

Define dynamically laid-out structures with non-constant sizes of array fields -- this is used in many file formats and network protocol headers (i.e. the length of one field depends on the value of another):

.. code:: cpp

	dynamic struct FileHdr {
		...
		char m_authorName[strlen(m_authorName) + 1];
		char m_authorEmail[strlen(m_authorEmail) + 1];
		uint8_t m_sectionCount;
		SectionDesc m_sectionTable[m_sectionCount];
		...
	}

In Jancy you can describe a dynamic struct, overlap your buffer with a pointer to this struct and then access the fields at dynamic offsets normally, just like you do with regular C-structs:

.. code:: cpp

	FileHdr const* hdr = buffer;

	displayAuthorInfo(hdr.m_authorName, hdr.m_authorEmail);

	for (size_t i = 0; i < hdr.m_sectionCount; i++)
		processSection(hdr.m_sectionTable[i].m_offset, hdr.m_sectionTable[i].m_size);

You can write to dynamic structs, too -- just make sure you fill it sequentially from top to bottom. And yes, dynamically calculated offsets are cached, so there is no significant performance penalty for using this facility.

Dual Error Handling Model
~~~~~~~~~~~~~~~~~~~~~~~~~

Both throw-catch and error-code approaches have their domains of application. Why force developers to choose one or another at the API design stage?

In Jancy you can write methods which can be *both* error-checked and caught exceptions from -- depending on what is more convenient at each particular call-site!

.. code:: cpp

	class File {
		bool errorcode open(char const* fileName);
		close();
		alias dispose = close;
	}

Use *throw-catch* semantics:

.. code:: cpp

	void foo(File* file) {
		file.open("data.bin");
		file.write(hdr, sizeof(hdr));
		file.write(data, dataSize);
		...

	catch:
		print($"error: $!\n");

	finally:
		file.close();
	}

\...or do *error-code* checks where it works better:

.. code:: cpp

	void bar() {
		disposable File file;
		bool result = try file.open("data.bin");
		if (!result) {
			print($"can't open: $!\n");
			...
		}
		...
	}

On a side note, see how elegantly Jancy solves the problem of *deterministic resource release*? Create a type with a method (or an alias) named ``dispose`` -- and every ``disposable`` instance of this type will get ``dispose`` method called upon exiting the scope (no matter which exit route is taken, of course).

Dual Type Modifiers
~~~~~~~~~~~~~~~~~~~

Jancy introduces yet another cool feature called *dual type modifiers* -- i.e. modifiers which have *different meaning* depending on the context. One pattern dual modifiers apply really well to is *read-only fields*:

.. code:: cpp

	class C {
		int readonly m_readOnly;
		void foo();
	}

The ``readonly`` modifier's meaning depends on whether a call-site belongs to the *private-circle* of the namespace:

.. code:: cpp

	void C.foo() {
		m_readOnly = 10; // ok
	}

	void bar(C* c) {
		print($"c.m_readOnly = $(c.m_readOnly)\n"); // ok
		c.m_readOnly = 20; // error: cannot store to const-location
	}

No more writing dummy getters!

Another common pattern is a pointer field which *inherits mutability* from its container:

.. code:: cpp

	struct ListEntry {
		ListEntry cmut* m_next;
		variant m_value;
	}

The ``cmut`` modifier must be used on the type of a member -- field, method, property. The meaning of ``cmut`` then depends on whether the container is *mutable*:

.. code:: cpp

	void bar(
		ListEntry* a,
		ListEntry const* b
	) {
		a.m_next.m_value = 10; // ok
		b.m_next.m_value = 10; // error: cannot store to const-location
	}

Implementing the equivalent functionality in C++ would require *a private field and three accessors*!

Finally, the most obvious application for dual modifiers -- *event fields*:

.. code:: cpp

	class C1 {
		event m_onCompleted();
		void work();
	}

The ``event`` modifier limits access to the methods of the underlying ``multicast`` depending on whether a call-site belongs to the *private-circle* of the namespace:

.. code:: cpp

	void C.work() {
		...
		m_onCompleted(); // ok
	}

	void foo(C* c) {
		c.m_onCompleted += onCompleted; // adding/remove handlers is ok
		c.m_onCompleted(); // error: non-friends can't fire events
	}

Other Notable Features
----------------------

* Multiple inheritance
* Properties -- the most comprehensive implementation thereof!
* Weak events (which do not require to unsubscribe)
* Partial application for functions and properties
* Function redirection
* Extension namespaces
* Thread local storage
* Bitflag enums
* Big-endian integers
* Perl-style formatting
* Hexadecimal, raw and multi-line literals
* Opaque classes
* break<n>, continue<n>

...and many other cool and often unique features, which simply can't be covered in the quick intro.

Documentation
-------------

* `Jancy Language Manual <https://vovkos.github.io/jancy/language>`_
* `Jancy Standard Library Reference <https://vovkos.github.io/jancy/stdlib>`_
* `Jancy C API Reference <https://vovkos.github.io/jancy/api>`_
* `Jancy Compiler Overview <https://vovkos.github.io/jancy/compiler>`_
* `Jancy Grammar Reference <https://vovkos.github.io/jancy/grammar>`_
* `Jancy Build Guide <https://vovkos.github.io/jancy/build-guide>`_
