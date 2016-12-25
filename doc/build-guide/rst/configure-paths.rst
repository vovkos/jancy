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

.. expand-macro:: paths-cmake Jancy

.. code-block:: bash

	LUA_INC_DIR         # path to Lua C include directory
	LUA_LIB_DIR         # path to Lua library directory
	LUA_LIB_NAME        # name of Lua library (lua/lua51/lua52/lua53)
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

.. expand-macro:: dependencies-cmake Jancy

You *do* need to specify LLVM paths (unless you build and install LLVM 3.4.2 and not the newer versions available in repositories).

On Windows, you will also need to specify paths to the required libraries -- they are unlikely to be found automatically.

.. rubric:: Sample paths.cmake on Linux:

.. code-block:: cmake

	set (LLVM_VERSION 3.4.2)

	set (LLVM_INC_DIR  /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/include)

	if ("${AXL_CPU}" STREQUAL "amd64")
		set (LLVM_CMAKE_DIR  /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-amd64/${CONFIGURATION_SUFFIX}/share/llvm/cmake)
		set (LLVM_INC_DIR    ${LLVM_INC_DIR} /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-amd64/${CONFIGURATION_SUFFIX}/include)
		set (LLVM_LIB_DIR    /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-amd64/${CONFIGURATION_SUFFIX}/lib)
	else ()
		set (LLVM_CMAKE_DIR  /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-x86/${CONFIGURATION_SUFFIX}/share/llvm/cmake)
		set (LLVM_INC_DIR    ${LLVM_INC_DIR} /home/vladimir/Develop/llvm/llvm-${LLVM_VERSION}/build/make-x86/${CONFIGURATION_SUFFIX}/include)
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

