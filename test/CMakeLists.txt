#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

add_subdirectory (test_jnc)

if (WIN32)
	option (
		BUILD_TEST_MFC
		"Build test_mfc"
		OFF
		)

	if (BUILD_TEST_MFC)
		add_subdirectory (test_mfc)
	endif ()
endif ()

if (${QT_FOUND})
	option (
		BUILD_TEST_QT
		"Build test_qt"
		ON
		)

	if (BUILD_TEST_QT)
		add_subdirectory (test_qt)
	endif ()
endif ()

#...............................................................................
