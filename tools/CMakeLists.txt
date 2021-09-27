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

if(QT_FOUND)
	option(
		BUILD_JNC_MINI_IDE
		"Build Jancy Mini-IDE"
		ON
	)

	if(BUILD_JNC_MINI_IDE)
		add_subdirectory(jnc_mini_ide)
	endif()
endif()

add_subdirectory(sign-jncx)

#...............................................................................
