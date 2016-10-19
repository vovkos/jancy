# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

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
