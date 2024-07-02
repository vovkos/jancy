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

Abstract
--------

.. raw:: html

    <a href="http://jancy.org"><img src="doc/mascot/jancy-new-200x380.jpg" alt="Jancy" align="right"></a>

Jancy is *the first and only* scripting language with **safe pointer arithmetics**, high level of ABI and source **compatibility with C**, spreadsheet-like **reactive programming**, fast **regex switches** applicable to streams, **dynamic layouts** for parsing dynamically structured binary blobs, **dual error handling** model that allows you to choose *error-code* or *exception* semantics at each *call-site*, and a lot of other useful features.

Design Principles
-----------------

* Statically typed C-family safe scripting language for IO and UI domains

    Jancy was born as a scripting language for `IO Ninja <https://ioninja.com>`__ -- a scriptable terminal/sniffer/protocol analyzer. As such, Jancy is specifically designed for safe and efficient handling of asynchronous IO and creating responsive user interfaces.

* High level of ABI and source compatibility with C

    Calling from Jancy to native code and vice versa is as *easy and efficient* as it gets. So is developing Jancy libraries in C/C++ and Jancy bindings to popular libraries. So is porting publicly available algorithms from C to Jancy -- *copy-paste* often suffices!

* Automatic memory management via accurate GC

    Losing manual memory management (together with the vast class of bugs and leaks associated with it) in favor of the GC employment has its price, but for a safe scripting language, it's 100% worth it.

* LLVM as a back-end

    This was a no-brainer from the very beginning. I started with LLVM 3.1 years ago. Now, Jancy builds and runs with any LLVM starting from 3.4.2 (the latest LLVM that still builds on MSVC 10) all the way up to the latest and greatest LLVM 18.

Key Features
------------

Safe Pointers and Pointer Arithmetic
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use C-structs and pointer arithmetic -- an *elegant and efficient* way of parsing and generating binary data -- and do so without worrying about buffer overruns and other pointer-related issues!

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

If bounds checks on a pointer access fail, Jancy runtime will throw an exception which you can handle as you like.

You can also safely pass "foreign" buffers from C/C++ to Jancy without creating a copy on the GC heap:

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
        JNC_END_CALL_SITE() // here ptr is invalidated and Jancy won't be able to access it
    }

Spreadsheet-like Reactive Programming
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Write auto-evaluating *formulas* just like you do in Excel -- but stay in complete control of when to use this Excel-like approach:

.. code:: cpp

    reactor m_uiReactor {
        m_title = $"Target address: $(m_addressCombo.m_editText)";
        m_localAddressProp.m_isEnabled = m_useLocalAddressProp.m_isChecked;
        m_isTransmitEnabled = m_state == State.Connected;
        ...

        onevent m_connectButton.m_onClicked() {
            // initiate connect
        }
    }

    m_uiReactor.start();
    // reactor will run and subscribe to all relevant UI properties and events;
    // from now on, all those UI events will be automatically handled by the reactor

    m_uiReactor.stop();
    // ...but not anymore

Using those UI properties and events *outside* of a ``reactor`` doesn't cause automatic subscribing (and associated side effects). So, depending on the developer's needs, the same class can act *reactive* or *non-reactive*.

All this, together with the developed infrastructure of *properties* and *events*, is perfect for UI programming!

Dynamic Layouts
~~~~~~~~~~~~~~~

For most real-world protocols and file formats, the binary data structure is not static — it heavily depends on what's inside. There could be variable-length or variable-type data fields, optional sections could be present or omitted, auxiliary sections could be added, etc.

Obviously, such data can't be defined using traditional C-like structures. To combat that, Jancy provides a facility called *dynamic layouts* specifically for describing *dynamic binary structures*:

.. code:: cpp

        jnc.DynamicLayout layout(p, size); // initialize a layout object
        ...
        dylayout (layout) {
            dyfield MyProtoHdr hdr; // map ProtoHdr as a dynamic field
            switch (hdr.m_command) { // depending on m_command, define follow-up fields
            case MyProtoCmd.Cmd1:
                dyfield MyProtoCmd1 cmd; // Cmd1-specific data
                break;

            case MyProtoCmd.Cmd2:
                dyfield MyProtoCmd2 cmd; // Cmd2-specific data
                dyfield char payload[cmd.m_dataLength]; // dynamically sized array
                break;

            // ...and so on
            }
        }

After defining a "specification" for your dynamic binary format like that, parsing a binary blob becomes a matter of feeding it to the ``dylayout`` statement.

It's also possible to place ``dylayout`` into an ``async`` coroutine -- then you'll be able to feed data chunk-by-chunk. Buffering and pausing when more bytes are required will happen automatically.

.. code:: cpp

    async layoutMyProto(jnc.DynamicLayout* layout) {
        dylayout (layout) {
            ...
        }
    }
    ...
    // initialize a stream layout and give it the first data chunk
    jnc.DynamicLayout layout(jnc.DynamicLayoutMode.Stream, p, size);
    ...
    layoutMyProto(layout);
    while (layout.m_isIncomplete) {
        // packet is incomplete -- wait for more data
        ...
        // when data arrives, resume the coroutine
        size_t sizeTaken = layout.resume(next, nextSize);
        ...
    }

    printf("packet is complete (%d bytes)\n", layout.m_size);
    // process the packet and continue to the next one

After parsing is completed, you can enumerate and walk over all discovered fields -- but of course, all the necessary actions can also be done from within ``dylayout``, as you parse.

Regex Switches and Match Operators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create fast and convenient regex-based ``switch`` and ``if`` statements to match text (or binary) data and extract important tidbits:

.. code:: cpp

    string_t text = readText();
    switch (text) {
    case "foo":
        ...
        break;
    case r"bar(\d+)":
        print($"bar id: $1\n");
        ...
        break;
    case r"\s+":
        // ignore whitespace
        break;
    ...
    default:
        print("mismatch\n");
    }
    ...
    text = readMoreText();
    if (text =~ r"([^s]+)\s*=\s*([^s]+)") {
        print($"match: key: $1, value: $2");
        ...
    }

To reference sub-matches, use the convenient pseudo-variables ``$0``, ``$1``, etc. -- just like in good-old Perl.

Under the hood, there's a heavily modified fork of Google's RE2 engine -- the best DFA-based regex engine out there. A small note on the "heavily modified" part. Unlike the original RE2, the engine used in Jancy is applicable to streams! What that means is that you can feed data to a regex switch chunk by chunk:

.. code:: cpp

    // initialize a jnc.RegexState object to store the DFA state between data chunks
    jnc.RegexState state;
    ...
    // when we have the next data chunk, feed it to the regex switch
    switch (state, string_t(p, size)) {
    case "open":
        printf("open at stream offset %llx (%zd bytes)\n", $0.m_offset, $0.m_length);
        // note that when a match is scattered across multiple chunks
        // you'll have no access to the match text $0.m_text or sub-matches $1, $2, etc.
        break;
    case "close":
        ....
        break;
    ...
    default:
        switch (state.m_lastExecResult) {
        case jnc.RegexExecResult.Mismatch:
            printf("recognition error\n");
            return -1;
        case jnc.RegexExecResult.Continue:
            // still somewhere inside the regex DFA, more data needed
            // feed the upcoming data to the regex switch
            break;
        case jnc.RegexExecResult.ContinueBackward:
            printf("match end found at %llx\n", $0.m_endOffset);
            // the start of match is before the last chunk (p, size)
            // feed the preceding data to the regex switch to find it
        }
    }

Scheduled Function Pointers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Schedulers* allow you to elegantly place the execution of a *callback* (completion routine, event handler, etc) in the correct environment -- for example, into the context of a specific thread:

.. code:: cpp

    class WorkerThread: jnc.Scheduler {
        override schedule(function* f()) {
            // enqueue f and signal worker thread event
        }
        ...
    }

Apply a binary operator ``@`` (read *"at"*) to create a *scheduled* pointer to your callback:

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

Jancy introduces yet another cool feature called *dual type modifiers* -- i.e. modifiers that have *different meaning* depending on the context. One pattern dual modifiers apply really well to is *read-only fields*:

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
        variant_t m_value;
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
