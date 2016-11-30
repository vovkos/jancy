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

paths.cmake
===========

Jancy build system uses ``paths.cmake`` file as the main reference when it needs to find a certain tool or library. When a path is not specified, a fallback attempt to find it using ``find_package`` will be made.

This allows for out-of-the-box *default* build and at the same time provides a *fine-grained* control over locations of dependencies. Here in Tibbo we have multiple versions of tools and libraries installed on the single build machine and at the same time we are always in full control over which tool or library is going to be used when building a particular project.

``paths.cmake`` files are **cascading**. It means, you can place one *anywhere* above the current directory and it will be found and used. From there you can chain-include the next ``paths.cmake`` and so on. This way you can specify some default locations for *all* your projects but still be able to override the paths for sub-projects.

Being machine-specific ``paths.cmake`` files are added to ``.gitignore`` and are never tracked in Git. Therefore, you need to write ``paths.cmake`` file as the very first step of configuration process. So, what should be inside?

To answer this question, you need to check ``dependencies.cmake`` file. Inside this file, a variable called ``AXL_PATH_LIST`` contains all the paths that will be used during the build. For ``jancy_b`` package this list looks like this:

.. code-block:: bash

	LUA_INC_DIR         # path to Lua C include directory
	LUA_LIB_DIR         # path to Lua library directory
	LUA_LIB_NAME        # name of lua library (lua51/lua52/lua53)
	LLVM_INC_DIR        # path to LLVM include directory (list is OK)
	LLVM_LIB_DIR        # path to LLVM library directory
	LLVM_CMAKE_DIR      # path to LLVM CMake module directory
	PCAP_INC_DIR        # (optional) path to Pcap include directory
	PCAP_LIB_DIR        # (optional) path to Pcap library directory
	LIBSSH2_INC_DIR     # (optional) path to LibSSH2 include directory
	LIBSSH2_LIB_DIR     # (optional) path to LibSSH2 library directory
	OPENSSL_INC_DIR     # (optional) path to OpenSSL include directory
	OPENSSL_LIB_DIR     # (optional) path to OpenSSL library directory
	QT_CMAKE_DIR        # (optional) path to QT CMake module directory
	QT_DLL_DIR          # (optional) path to QT dynamic library directory (Windows only)
	RAGEL_EXE           # path to Ragel executable
	7Z_EXE              # path to 7-Zip executable
	DOXYGEN_EXE         # (optional) path to Doxygen executable
	DOXYREST_CMAKE_DIR  # (optional) path to Doxyrest CMake module directory
	SPHINX_BUILD_EXE    # (optional) path to Sphinx compiler executable sphinx-build
	PDFLATEX_EXE        # (optional) path to Latex-to-PDF compiler

Note that it you don't necessarily have to specify each and every variable above.

First of all, it's OK to completely omit *optional* dependencies -- if you don't need those.

Secondly, required dependencies may be auto-detected -- on Unix systems installed libraries and tools will likely be found automatically. On Windows Jancy build system will automatically find executables if they are added to ``PATH`` (via ``where`` command as a fallback when a path to the executable is not specified).

You do need to specify LLVM paths (unless you build and install LLVM 3.4.2 and not the newer versions available in repositories).

On Windows, you will also need to specify paths to the required libraries -- they are unlikely to be found automatically.

And of course, you can always use ``paths.cmake`` to *fine-tune* the location of a specific tool/library.

I personally prefer to always specify all the paths explicitly.

.. rubric:: Sample paths.cmake on Linux:

.. code-block:: cmake

	set (LLVM_VERSION 3.4.2)

	set (LLVM_INC_DIR  /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/include)

	if ("${AXL_CPU}" STREQUAL "amd64")
		set (LLVM_CMAKE_DIR  /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-amd64/${CONFIGURATION_SUFFIX}/share/llvm/cmake)
		set (LLVM_INC_DIR    ${LLVM_INC_DIR} "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-amd64/${CONFIGURATION_SUFFIX}/include)
		set (LLVM_LIB_DIR    /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-amd64/${CONFIGURATION_SUFFIX}/lib)
	else ()
		set (LLVM_CMAKE_DIR  /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-x86/${CONFIGURATION_SUFFIX}/share/llvm/cmake)
		set (LLVM_INC_DIR    ${LLVM_INC_DIR} "/home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-x86/${CONFIGURATION_SUFFIX}/include)
		set (LLVM_LIB_DIR    /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-x86/${CONFIGURATION_SUFFIX}/lib)
	endif ()

.. rubric:: Sample paths.cmake on Windows:

.. code-block:: cmake

	set (LUA_VERSION   5.2.1)
	set (LUA_LIB_NAME  lua52)
	set (LLVM_VERSION  3.4.2)
	set (RAGEL_VERSION 6.7)

	set (7Z_EXE       "c:/Program Files/7-Zip/7z.exe")
	set (PERL_EXE     c:/Develop/ActivePerl/bin/perl.exe)
	set (RAGEL_EXE    c:/Develop/ragel/ragel-${RAGEL_VERSION}/ragel.exe)
	set (LUA_INC_DIR  c:/Develop/lua/lua-${LUA_VERSION}/include)
	set (LLVM_INC_DIR c:/Develop/llvm/llvm-${LLVM_VERSION}/include)

	if ("${AXL_CPU}" STREQUAL "amd64")
		set (LUA_LIB_DIR    c:/Develop/lua/lua-${LUA_VERSION}/lib/amd64/${CONFIGURATION_SUFFIX})
		set (LLVM_INC_DIR   ${LLVM_INC_DIR} c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10-amd64/include)
		set (LLVM_LIB_DIR   c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10-amd64/lib/${CONFIGURATION_SUFFIX})
		set (LLVM_CMAKE_DIR c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10-amd64/share/llvm/cmake)
	else ()
		set (LUA_LIB_DIR    c:/Develop/lua/lua-${LUA_VERSION}/lib/x86/${CONFIGURATION_SUFFIX})
		set (LLVM_INC_DIR   ${LLVM_INC_DIR} c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10/include)
		set (LLVM_LIB_DIR   c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10/lib/${CONFIGURATION_SUFFIX})
		set (LLVM_CMAKE_DIR c:/Develop/llvm/llvm-${LLVM_VERSION}/build/msvc10/share/llvm/cmake)
	endif()

