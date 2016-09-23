# This file is part of AXL (R) library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

set (DOC_NAME      "Jancy Language Manual")
set (TEX_FILE_NAME jancy_language.tex)
set (PDF_FILE_NAME jancy_language.pdf)

set (RST_DIR  ${CMAKE_CURRENT_LIST_DIR}/rst)
set (HTML_DIR ${CMAKE_CURRENT_BINARY_DIR}/html)
set (TEX_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tex)
set (PDF_DIR  ${CMAKE_CURRENT_BINARY_DIR}/pdf)
set (TMP_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tmp)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (
	SPHINX_HTML_CMD_LINE
	-b html
	-d ${TMP_DIR}/sphinx-doctrees
	-c ${CMAKE_CURRENT_BINARY_DIR}
	${RST_DIR}
	${HTML_DIR}
	)

set (
	SPHINX_TEX_CMD_LINE
	-b latex
	-d ${TMP_DIR}/sphinx-doctrees
	-c ${CMAKE_CURRENT_BINARY_DIR}
	${RST_DIR}
	${TEX_DIR}
	)

set (
	PDFLATEX_MIKTEX_CMD_LINE
	-quiet
	-output-directory=${PDF_DIR}
	-aux-directory=${TMP_DIR}
	${TEX_FILE_NAME}
	)

set (
	PDFLATEX_TEXLIVE_CMD_LINE
	-interaction=batchmode
	-output-directory=${TMP_DIR}
	${TEX_FILE_NAME}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (
	CMD_LINE_VAR_LIST

	SPHINX_HTML_CMD_LINE
	SPHINX_TEX_CMD_LINE
	PDFLATEX_MIKTEX_CMD_LINE
	PDFLATEX_TEXLIVE_CMD_LINE
	)

foreach (_VAR ${CMD_LINE_VAR_LIST})
	string (
		REPLACE  
		";" " "
		${_VAR}
		"${${_VAR}}"
		)
endforeach ()

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

configure_file (
	conf.py.in
	conf.py
	)

if (WIN32)	
	configure_file (
		build-html.bat.in
		build-html.bat
		)

	configure_file (
		build-pdf.bat.in
		build-pdf.bat
		)		
else ()
	axl_configure_file_w_permissions (
		build-html.sh.in
		build-html.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions (
		build-pdf.sh.in
		build-pdf.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif ()
	
#..............................................................................