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

Samples
=======

After Jancy is successfully built and tested, you should probably check out the ``./samples`` folder.

* ``jnc``

	This sub-folder contains multiple Jancy source files demonstrating most prominent language features.

* ``jnc_sample_01_embed_c``

	This sample demonstrates how to embed Jancy as a scripting language into a pure C program. The sample also shows how to implement Jancy classes in C and how to export them into the namespace of the script, thus providing two-way interaction between the script and the host application.

	Run it as::

		jnc_sample_01_embed_c <path-to-script>

	Sample script could be found at: ``./samples/jnc_sample_01_embed_c/script.jnc``

* ``jnc_sample_02_embed_cpp``

	This sample is a C++ counterpart of the previous sample. It demonstrates how to embed Jancy as a scripting language into C++ program and how to implement Jancy classes in C++.

	Run it as::

		jnc_sample_02_embed_cpp <path-to-script>

	Sample script could be found at: ``./samples/jnc_sample_02_embed_cpp/script.jnc``

* ``jnc_sample_03_dialog``

	This sample demonstrates how to create a Jancy binding to QT widget classes and how to apply Jancy reactive programming concepts to UI programming.

	Obviously, this sample requires QT to be built and run.

	Run it as::

		jnc_sample_03_dialog <path-to-script>

	Sample script could be found at: ``./samples/jnc_sample_03_dialog/script.jnc``

You can also check out ``./test/test_qt`` -- a Jancy editor capable of compiling and running user code.

This QT application is constantly used by us for troubleshooting and bug-fixing of Jancy compiler, Jancy runtime and Jancy standard library. What makes this application really convenient for the said purpose is the blend of *editor*, *compiler* and *runtime* -- all in confines of the single process, which makes everything as *debugger-friendly* as possible.