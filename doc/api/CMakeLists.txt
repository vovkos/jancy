# This file is part of AXL (R) library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

set (XML_DIR  ${CMAKE_CURRENT_BINARY_DIR}/xml)
set (RST_DIR  ${CMAKE_CURRENT_BINARY_DIR}/rst)
set (HTML_DIR ${CMAKE_CURRENT_BINARY_DIR}/html)
set (TEX_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tex)
set (PDF_DIR  ${CMAKE_CURRENT_BINARY_DIR}/pdf)
set (TMP_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tmp)

configure_file (
	Doxyfile.in
	Doxyfile
	)

configure_file (
	rst/index.rst
	${RST_DIR}/index.rst
	COPYONLY
	)

configure_file (
	conf.py.in
	conf.py
	)

if (WIN32)	
	set (SHELL_FILE_HDR "@echo off")
	set (SHELL_LINE_CONT "^")

	string (REPLACE "$(Configuration)" "%1" JANCY_EXE ${JANCY_EXE})
	string (REPLACE "$(Configuration)" "%1" DOXYREST_EXE ${DOXYREST_EXE})
	
	configure_file (
		build-rst.in
		build-rst.bat
		)

	configure_file (
		build-html.in
		build-html.bat
		)

	configure_file (
		build-pdf.in
		build-pdf.bat
		)		
else ()
	set (SHELL_FILE_HDR "#!/bin/bash")
	set (SHELL_LINE_CONT "\\")

	axl_configure_file_w_permissions (
		build-rst.in
		build-rst.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions (
		build-html.in
		build-html.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions (
		build-pdf.in
		build-pdf.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif ()
	
#..............................................................................
