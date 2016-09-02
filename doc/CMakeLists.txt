# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

if (SPHINX_FOUND)
	add_subdirectory (language)

	if (DOXYREST_FOUND)
		add_subdirectory (stdlib)

		if (DOXYGEN_FOUND)	
			add_subdirectory (api)
		endif ()
	endif ()
endif ()

#..............................................................................
