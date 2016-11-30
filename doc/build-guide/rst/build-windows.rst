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

	Building on Windows
	===================

	To build Jancy on Windows you can start Visual Studio by double-clicking the newly generated solution file ``jancy_b.sln`` and build it from the IDE.

	If you prefer building from the command line, run in ``./build`` folder::

		cmake --build .

	You can pass extra command-line arguments to ``msbuild.exe`` using ``--`` switch. For example, if you want to have a multi-core build of ``Release`` configuration, run::

		cmake --build . -- /maxcpucount /property:configuration=Release

	Please refer to ``msbuild`` documentation for more details on command-line parameters: https://msdn.microsoft.com/en-us/library/ms164311.aspx

	After Jancy build is complete you will have Jancy static library files in ``./build/jancy/lib/$(Configuration)`` directory; command line compiler, dynamic extension libraries, and sample executable binaries can be found in ``./build/jancy/bin/$(Configuration)``.

	Testing
	-------

	Now let's test compiled binaries.

	If you used Visual Studio IDE to build Jancy, you can run tests from inside the IDE. To do so, right-click on ``RUN_TESTS`` pseudo-project in Visual Studio Solution Explorer/Navigator and select ``Build``.

	If you prefer using the command line, then run in ``./build`` folder::

		ctest -C Debug
		ctest -C Release

	Another way to run tests on Windows is::

		cmake --build . --target RUN_TESTS --config Debug
		cmake --build . --target RUN_TESTS --config Release

	Obviously, you can only test ``Debug`` or ``Release`` configuration if you have already built this configuration.

	If everything went smooth, you should see something like::

		100% tests passed, 0 tests failed out of 123

	Congratulations! You have just successfully built Jancy.

