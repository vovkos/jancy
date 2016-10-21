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

if (SPHINX_FOUND)
	add_subdirectory (language)

	if (DOXYREST_FOUND)
		add_subdirectory (stdlib)

		if (DOXYGEN_FOUND)
			add_subdirectory (api)
		endif ()
	endif ()
endif ()

#...............................................................................
