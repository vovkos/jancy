# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

if (WIN32)
	add_subdirectory (windows)
elseif (UNIX)
	add_subdirectory (linux)
endif ()

#..............................................................................
