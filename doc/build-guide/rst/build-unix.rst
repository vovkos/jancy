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

Building on Unix
================

Run in ``./build`` folder::

	make

You might also want to add ``-j <n>`` to make use of multiple CPU cores and speed up build process, like::

	make -j 4

After Jancy build is complete you will have Jancy static library files in ``./build/jancy/lib/${CMAKE_BUILD_TYPE}`` directory; command line compiler, dynamic extension libraries and sample executable binaries can be found in ``./build/jancy/bin/${CMAKE_BUILD_TYPE}``.

Note that with ``make``-based build changing configuration from ``Debug`` to ``Release`` should be done at CMake configuration step (Xcode and Visual Studio are multi-configuration build systems).

Testing
-------

Now let's test compiled binaries.

Run in ``./build`` folder::

	make test

If everything went smooth, you should see something like::

	100% tests passed, 0 tests failed out of 123

Congratulations! You have just successfully built Jancy.