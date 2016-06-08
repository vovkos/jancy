# This file is part of AXL (R) library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

configure_file (
	Doxyfile.in
	Doxyfile
	)

if (WIN32)	
	set (ECHO_OFF "@echo off")
	
	configure_file (
		build-doxygen.in
		build-doxygen.bat
		)
else ()
	axl_configure_file_w_permissions (
		build-doxygen.sh.in
		build-doxygen.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif ()
	
#..............................................................................
