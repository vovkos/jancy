# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

if (WIN32)
	option (
		BUILD_JNC_MFC_TEST
		"Build JncMfcTest"
		OFF
		)

	if (BUILD_JNC_MFC_TEST)
		add_subdirectory (JncMfcTest)
	endif ()
endif ()

#..............................................................................

if (${QT_FOUND})
	option (
		BUILD_JNC_QT_TEST
		"Build JncQtTest"
		ON
		)

	if (BUILD_JNC_QT_TEST)
		add_subdirectory (JncQtTest)
	endif ()
endif ()

#..............................................................................
