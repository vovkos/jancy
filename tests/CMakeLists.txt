# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

root_virtual_folder (${CMAKE_CURRENT_BINARY_DIR}/tests.virtual)

#..............................................................................

if (WIN32)
	option (
		AXL_BUILD_JNC_MFC_TEST
		"Build JncMfcTest"
		OFF
		)

	if (AXL_BUILD_JNC_MFC_TEST)
		add_subdirectory (JncMfcTest)
	endif ()
endif ()

#..............................................................................

if (${QT_FOUND})
	option (
		AXL_BUILD_JNC_QT_TEST
		"Build JncQtTest"
		ON
		)

	if (AXL_BUILD_JNC_QT_TEST)
		add_subdirectory (JncQtTest)
	endif ()
endif ()

#..............................................................................
