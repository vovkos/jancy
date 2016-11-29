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

::

	LUA_INC_DIR
	LUA_LIB_DIR
	LUA_LIB_NAME
	LLVM_INC_DIR
	LLVM_LIB_DIR
	LLVM_CMAKE_DIR
	PCAP_INC_DIR
	PCAP_LIB_DIR
	LIBSSH2_INC_DIR
	LIBSSH2_LIB_DIR
	OPENSSL_INC_DIR
	OPENSSL_LIB_DIR
	QT_CMAKE_DIR
	QT_DLL_DIR
	RAGEL_EXE
	7Z_EXE
	DOXYGEN_EXE
	DOXYREST_CMAKE_DIR
	SPHINX_BUILD_EXE
	PDFLATEX_EXE

Note that it you don't have to necessarily specify each and every one of them. First of all, if this is an *optional* dependency and you don't need it -- you can safely omit it. But even with *required* dependencies you should only add an entry to ``paths.cmake`` when you need to *fine-tune* the location for a specific tool/library. If the path to a tool/library is auto-detected correctly -- you don't have to touch it.

On Windows, however, you *DO NEED* to specify the paths to the required libraries -- they are unlikely to be found automatically. It's easier with tools, though -- if an executable is available via ``PATH``, it will be found (Jancy build system uses ``where`` command as a fallback when path to executable is not specified). Still, I prefer to always specify paths explicitly.

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

