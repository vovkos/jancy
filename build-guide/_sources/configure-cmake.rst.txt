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

CMake Configuration Step
========================

.. expand-macro:: cmake-configure jancy_b

.. rubric:: Sample Linux output:

::

	AXL CMake:
		Invoked from:        /home/vladimir/Projects/jancy_b/CMakeLists.txt
		dependencies.cmake:  /home/vladimir/Projects/jancy_b/dependencies.cmake
		settings.cmake:      /home/vladimir/Projects/jancy_b/settings.cmake
		paths.cmake:         /home/vladimir/Projects/paths.cmake
		Target CPU:          amd64
		Build configuration: Debug
	C/C++:
		C Compiler:          /usr/bin/cc
		C flags (Debug):     -m64 -mcx16 -fPIC -fvisibility=hidden -Wno-multichar -g
		C flags (Release):   -m64 -mcx16 -fPIC -fvisibility=hidden -Wno-multichar -O3 -DNDEBUG
		C++ Compiler:        /usr/bin/c++
		C++ flags (Debug):   -m64 -mcx16 -fno-rtti -fPIC -fvisibility=hidden -Wno-multichar -g
		C++ flags (Release): -m64 -mcx16 -fno-rtti -fPIC -fvisibility=hidden -Wno-multichar -O3 -DNDEBUG
	Dependency path definitions:
		LLVM_CMAKE_DIR:      /home/vladimir/Develop/llvm/llvm-3.4.2/build/make-amd64/Debug/share/llvm/cmake
		LLVM_INC_DIR:        /home/vladimir/Develop/llvm/llvm-3.4.2/include;/home/vladimir/Develop/llvm/llvm-3.4.2/build/make-amd64/Debug/include
		LLVM_LIB_DIR:        /home/vladimir/Develop/llvm/llvm-3.4.2/build/make-amd64/Debug/lib
	Lua paths:
		Includes:            /usr/include
		Library dir:         /usr/lib64
		Library name:        lua
	LLVM paths:
		CMake files:         /home/vladimir/Develop/llvm/llvm-3.4.2/build/make-amd64/Debug/share/llvm/cmake
		Includes:            /home/vladimir/Develop/llvm/llvm-3.4.2/include;/home/vladimir/Develop/llvm/llvm-3.4.2/build/make-amd64/Debug/include
		Libraries:           /home/vladimir/Develop/llvm/llvm-3.4.2/build/make-amd64/Debug/lib
	Ragel found at:          /usr/bin/ragel
	Perl found at:           /usr/bin/perl
	7-Zip found at:          /usr/bin/7z
	Pcap paths:
		Includes:            /usr/include
		Libraries:           /usr/lib64
	LibSSH2 paths:
		Includes:            /usr/include
		Libraries:           /usr/lib64
	OpenSSL paths:
		Includes:            /usr/include
		Libraries:           /usr/lib64
	Doxygen found at:        /usr/bin/doxygen
	doxyrest:                NOT FOUND, adjust AXL_IMPORT_DIR_LIST in dependencies.cmake (optional)
	Sphinx found at:         /usr/bin/sphinx-build
	Pdflatex found at:       /usr/bin/pdflatex
	QT paths:
		Core CMake files:    /usr/lib64/cmake/Qt5Core
		Gui CMake files:     /usr/lib64/cmake/Qt5Gui
		Widgets CMake files: /usr/lib64/cmake/Qt5Widgets
		Network CMake files: /usr/lib64/cmake/Qt5Network
	AXL paths:
		CMake files:         /home/vladimir/Projects/jancy_b/axl/cmake;/home/vladimir/Projects/jancy_b/.build/axl/cmake
		Includes:            /home/vladimir/Projects/jancy_b/axl/include
		Libraries:           /home/vladimir/Projects/jancy_b/.build/axl/lib/Debug
	Graco paths:
		CMake files:         /home/vladimir/Projects/jancy_b/graco/cmake;/home/vladimir/Projects/jancy_b/.build/graco/cmake
		Includes:            /home/vladimir/Projects/jancy_b/graco/include
		Frames:              /home/vladimir/Projects/jancy_b/graco/frame
		Executable:          /home/vladimir/Projects/jancy_b/.build/graco/bin/Debug/graco
	Configuring done

.. rubric:: Sample Windows output:

::

	AXL CMake:
		Invoked from:        C:/Projects/repos/jancy_b/CMakeLists.txt
		dependencies.cmake:  C:/Projects/repos/jancy_b/dependencies.cmake
		settings.cmake:      C:/Projects/repos/jancy_b/settings.cmake
		paths.cmake:         C:/Projects/repos/paths.cmake
		Target CPU:          amd64
	C/C++:
		C Compiler:          C:/Develop/msvs/msvs2010/VC/bin/x86_amd64/cl.exe
		C flags (Debug):     /DWIN32 /D_WINDOWS /W3 /EHsc /GR /D_DEBUG /Ob0 /Od /RTC1 /MTd /Zi
		C flags (Release):   /DWIN32 /D_WINDOWS /W3 /EHsc /GR /O2 /Ob2 /D NDEBUG /MT /Zi
		C++ Compiler:        C:/Develop/msvs/msvs2010/VC/bin/x86_amd64/cl.exe
		C++ flags (Debug):   /DWIN32 /D_WINDOWS /W3 /EHsc /GR /D_DEBUG /Ob0 /Od /RTC1 /MTd /Zi
		C++ flags (Release): /DWIN32 /D_WINDOWS /W3 /EHsc /GR /O2 /Ob2 /D NDEBUG /MT /Zi
		C/C++ definitions:   UNICODE;_UNICODE
	Dependency path definitions:
		7Z_EXE:              c:/Program Files/7-Zip/7z.exe
		DOXYGEN_EXE:         c:/Develop/doxygen/doxygen-1.8.11-x86/doxygen.exe
		LIBSSH2_INC_DIR:     c:/Develop/libssh2/libssh2-1.4.4/include
		LIBSSH2_LIB_DIR:     c:/Develop/libssh2/libssh2-1.4.4/lib/amd64/$(Configuration)
		LLVM_CMAKE_DIR:      c:/Develop/llvm/llvm-3.4.2/build/msvc10-amd64/share/llvm/cmake
		LLVM_INC_DIR:        c:/Develop/llvm/llvm-3.4.2/include;c:/Develop/llvm/llvm-3.4.2/build/msvc10-amd64/include
		LLVM_LIB_DIR:        c:/Develop/llvm/llvm-3.4.2/build/msvc10-amd64/lib/$(Configuration)
		LUA_INC_DIR:         c:/Develop/lua/lua-5.2.1/include
		LUA_LIB_DIR:         c:/Develop/lua/lua-5.2.1/lib/amd64/$(Configuration)
		LUA_LIB_NAME:        lua52
		OPENSSL_INC_DIR:     c:/Develop/openssl/openssl-win64-1.0.1h/include
		OPENSSL_LIB_DIR:     c:/Develop/openssl/openssl-win64-1.0.1h/lib/vc/static
		PCAP_INC_DIR:        c:/Develop/winpcap/winpcap-4.1.2/include
		PCAP_LIB_DIR:        c:/Develop/winpcap/winpcap-4.1.2/lib/x64
		PDFLATEX_EXE:        c:/Program Files (x86)/MiKTeX 2.9/miktex/bin/pdflatex.exe
		QT_CMAKE_DIR:        e:/Develop/qt/qt-5.3.2/build/amd64/qtbase/lib/cmake
		QT_DLL_DIR:          e:/Develop/qt/qt-5.3.2/build/amd64/qtbase/lib
		RAGEL_EXE:           c:/Develop/ragel/ragel-6.7/ragel.exe
		SPHINX_BUILD_EXE:    c:/Develop/ActivePython/Scripts/sphinx-build.exe
	Lua paths:
		Includes:            c:/Develop/lua/lua-5.2.1/include
		Library dir:         c:/Develop/lua/lua-5.2.1/lib/amd64/$(Configuration)
		Library name:        lua52
	LLVM paths:
		CMake files:         c:/Develop/llvm/llvm-3.4.2/build/msvc10-amd64/share/llvm/cmake
		Includes:            c:/Develop/llvm/llvm-3.4.2/include;c:/Develop/llvm/llvm-3.4.2/build/msvc10-amd64/include
		Libraries:           c:/Develop/llvm/llvm-3.4.2/build/msvc10-amd64/lib/$(Configuration)
	Pcap paths:
		Includes:            c:/Develop/winpcap/winpcap-4.1.2/include
		Libraries:           c:/Develop/winpcap/winpcap-4.1.2/lib/x64
	LibSSH2 paths:
		Includes:            c:/Develop/libssh2/libssh2-1.4.4/include
		Libraries:           c:/Develop/libssh2/libssh2-1.4.4/lib/amd64/$(Configuration)
	OpenSSL paths:
		Includes:            c:/Develop/openssl/openssl-win64-1.0.1h/include
		Libraries:           c:/Develop/openssl/openssl-win64-1.0.1h/lib/vc/static
	doxyrest:                NOT FOUND, adjust AXL_IMPORT_DIR_LIST in dependencies.cmake (optional)
	QT paths:
		Core CMake files:    e:/Develop/qt/qt-5.3.2/build/amd64/qtbase/lib/cmake/Qt5Core
		Gui CMake files:     e:/Develop/qt/qt-5.3.2/build/amd64/qtbase/lib/cmake/Qt5Gui
		Widgets CMake files: e:/Develop/qt/qt-5.3.2/build/amd64/qtbase/lib/cmake/Qt5Widgets
		Network CMake files: e:/Develop/qt/qt-5.3.2/build/amd64/qtbase/lib/cmake/Qt5Network
	AXL paths:
		CMake files:         C:/Projects/repos/jancy_b/axl/cmake;C:/Projects/repos/jancy_b/build/axl/cmake
		Includes:            C:/Projects/repos/jancy_b/axl/include
		Libraries:           C:/Projects/repos/jancy_b/build/axl/lib/$(Configuration)
	Graco paths:
		CMake files:         C:/Projects/repos/jancy_b/graco/cmake;C:/Projects/repos/jancy_b/build/graco/cmake
		Includes:            C:/Projects/repos/jancy_b/graco/include
		Frames:              C:/Projects/repos/jancy_b/graco/frame
		Executable:          C:/Projects/repos/jancy_b/build/graco/bin/$(Configuration)/graco
	Configuring done

.. expand-macro:: cmake-post-configure
