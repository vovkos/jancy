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

Libraries
=========

These libraries are **required** for building the Jancy compiler.

* LLVM 3.4.2

	Jancy uses LLVM as a back-end. LLVM is a collection of compiler libraries which quickly became a de-facto standard framework for compiler back-end implementation.

	Since Jancy and `IO Ninja <http://tibbo.com/ninja>`_ are still officially built on Visual Studio 2010, and newer versions of LLVM unfortunately **CANNOT** be built on Visual Studio 2010 (because of the lack of C++11 support) -- you will need to download and build LLVM 3.4.2 which is the latest LLVM version to still support Visual Studio 2010.

	Sources of LLVM 3.4.2 are available at: http://llvm.org/releases/download.html#3.4.2

	After downloading and extracting LLVM sources please follow the LLVM CMake build guide at: http://llvm.org/docs/CMake.html

	.. note::

		When building static LLVM libraries on Linux systems, it is **highly recommended** to add ``-fvisibility=hidden`` to C/C++ flags [#f1]_.

	.. note::

		On Linux systems it is also recommended to set ``LLVM_ENABLE_TERMINFO`` to ``OFF`` as to avoid unnecessary dependency on ``libncurses``.

	.. note::

		You don't have to *install* LLVM; merely building the static libraries is enough.

* Lua

	Grammar compiler Graco uses Lua string templates for generating C++ code from LL(k) grammar. Therefore, Lua headers and libraries are required for building Graco.

	.. expand-macro:: lua-common-info

These libraries are **optional** and are only required in order to build the full Jancy source package.

* OpenSSL & LibSSH2

	LibSSH2 and OpenSSL libraries are required to build ``jnc_io_ssh.jncx`` dynamic extension library. This library provides ``io.SshChannel`` class for managing client-side SSH connections.

	If this functionality is not required, neither OpenSSL nor LibSSH2 is necessary.

	Both OpenSSL and LibSSH2 are available in official repositories of most Linux distributions. Alternatively, they can be built from sources available at official websites.

	Official OpenSSL website: http://www.openssl.org
	Official LibSSH2 website: http://www.libssh2.org

	There are also a number of unofficial projects on the web offering pre-compiled libraries for both OpenSSL and LibSSH2.

* Pcap/WinPcap

	Pcap (called WinPcap on Windows) library is required to build ``jnc_io_pcap.jncx`` dynamic extension library. This library provides ``io.Pcap`` class for low-level network packet management including filtering, capturing and injecting network traffic.

	If this functionality is not required, Pcap library is not necessary.

	.. expand-macro:: pcap-common-info

* QT 5.x

	QT is a comprehensive cross-platform C++ framework. Jancy relies on QT to provide the user interface in its GUI-based tests and samples:

	- ``test_qt`` is a simple Jancy editor capable of compiling and running user code;
	- ``jnc_sample_03_dialog`` is a GUI sample demonstrating the application of Jancy reactive programming concepts to QT widgets.

	.. expand-macro:: qt-common-info

.. rubric:: Footnotes:

.. [#f1] Otherwise, the linker may add a subset of LLVM symbols to the ``.dynsym`` table of the resulting ``ELF`` executable. This may cause crashes due to conflicts with the system-installed LLVM. It may happen, for example, with QT applications on systems with **mesa OpenGL** library (QT creates a window and uses mesa for rendering; mesa uses shared LLVM for shader compilation)
