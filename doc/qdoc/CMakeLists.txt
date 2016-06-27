# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

configure_file (
	jnc_api.qdocconf.in
	jnc_api.qdocconf
	)

configure_file (
	jnc_std.qdocconf.in
	jnc_std.qdocconf
	)

if (WIN32)	
	set (ECHO_OFF "@echo off")
	
	configure_file (
		build-qdoc.in
		build-qdoc.bat
		)
else ()
	axl_configure_file_w_permissions (
		build-qdoc.in
		build-qdoc.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif ()
	
file (GLOB _FILE_LIST ${CMAKE_CURRENT_LIST_DIR}/style/*.*)
file (COPY ${_FILE_LIST} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/style/)
	
file (GLOB _FILE_LIST ${CMAKE_CURRENT_LIST_DIR}/images/*.*)
file (COPY ${_FILE_LIST} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images/)

#..............................................................................
