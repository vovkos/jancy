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

add_subdirectory (jnc_test_abi)
add_subdirectory (jnc)
add_subdirectory (ioninja)

if (WIN32)
	option (
		BUILD_JNC_TEST_MFC
		"Build jnc_test_mfc"
		OFF
		)

	if (BUILD_JNC_TEST_MFC)
		add_subdirectory (jnc_test_mfc)
	endif ()
endif ()

if (QT_FOUND)
	option (
		BUILD_JNC_TEST_QT
		"Build jnc_test_qt"
		ON
		)

	if (BUILD_JNC_TEST_QT)
		add_subdirectory (jnc_test_qt)
	endif ()
endif ()

#...............................................................................
