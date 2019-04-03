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

set(DOC_NAME "Jancy C API Reference")

set(TEX_BASE_NAME jancy-api-${JANCY_VERSION_FILE_SUFFIX})
set(TEX_FILE_NAME ${TEX_BASE_NAME}.tex)
set(PDF_FILE_NAME ${TEX_BASE_NAME}.pdf)

set(STATIC_RST_DIR ${CMAKE_CURRENT_LIST_DIR}/rst)

set(XML_DIR  ${CMAKE_CURRENT_BINARY_DIR}/xml)
set(RST_DIR  ${CMAKE_CURRENT_BINARY_DIR}/rst)
set(TEX_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tex)
set(TMP_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tmp)
set(HTML_DIR ${JANCY_DOC_DIR}/html/api)
set(PDF_DIR  ${JANCY_DOC_DIR}/pdf)

set(SPHINX_HTML_THEME ${AXL_SPHINX_HTML_THEME})

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set(
	DOXYGEN_CMD_LINE
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	)

set(XMLGEN_EXE ${DOXYGEN_EXE})
set(XMLGEN_CMD_LINE ${DOXYGEN_CMD_LINE})

set(
	DOXYREST_CMD_LINE
	${XML_DIR}/index.xml
	-D g_hasNewLineAfterReturnType
	-D g_excludeEmptyDefines
	-D g_excludeDefinePattern=__STDC_
	-D g_nameDelimiter=::
	-D g_introFile=intro.rst
	-o ${RST_DIR}/index.rst
	-F ${DOXYREST_FRAME_DIR}
	-f c_index.rst.in
	)

set(
	SPHINX_HTML_CMD_LINE
	-b html
	-d ${TMP_DIR}/sphinx-doctrees
	-c ${CMAKE_CURRENT_BINARY_DIR}
	${RST_DIR}
	${HTML_DIR}
	)

set(
	SPHINX_TEX_CMD_LINE
	-b latex
	-d ${TMP_DIR}/sphinx-doctrees
	-c ${CMAKE_CURRENT_BINARY_DIR}
	${RST_DIR}
	${TEX_DIR}
	)

set(
	PDFLATEX_MIKTEX_CMD_LINE
	-quiet
	-output-directory=${PDF_DIR}
	-aux-directory=${TMP_DIR}
	${TEX_FILE_NAME}
	)

set(
	PDFLATEX_TEXLIVE_CMD_LINE
	-interaction=batchmode
	-output-directory=${TMP_DIR}
	${TEX_FILE_NAME}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set(
	CMD_LINE_VAR_LIST

	XMLGEN_CMD_LINE
	DOXYREST_CMD_LINE
	SPHINX_HTML_CMD_LINE
	SPHINX_TEX_CMD_LINE
	PDFLATEX_MIKTEX_CMD_LINE
	PDFLATEX_TEXLIVE_CMD_LINE
	)

foreach(_VAR ${CMD_LINE_VAR_LIST})
	string(
		REPLACE
		";" " "
		${_VAR}
		"${${_VAR}}"
		)
endforeach()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

configure_file(
	Doxyfile.in
	Doxyfile
	)

configure_file(
	conf.py.in
	conf.py
	)

if(WIN32)
	string(REPLACE "$(Configuration)" "%1" DOXYREST_EXE "${DOXYREST_EXE}")

	configure_file(
		${AXL_DOC_SHARE_DIR}/sh/build-xml.bat.in
		build-xml.bat
		)

	configure_file(
		${AXL_DOC_SHARE_DIR}/sh/build-rst.bat.in
		build-rst.bat
		)

	configure_file(
		${AXL_DOC_SHARE_DIR}/sh/build-html.bat.in
		build-html.bat
		)

	configure_file(
		${AXL_DOC_SHARE_DIR}/sh/build-pdf.bat.in
		build-pdf.bat
		)
else()
	axl_configure_file_w_permissions(
		${AXL_DOC_SHARE_DIR}/sh/build-xml.sh.in
		build-xml.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions(
		${AXL_DOC_SHARE_DIR}/sh/build-rst.sh.in
		build-rst.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions(
		${AXL_DOC_SHARE_DIR}/sh/build-html.sh.in
		build-html.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions(
		${AXL_DOC_SHARE_DIR}/sh/build-pdf.sh.in
		build-pdf.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif()

#...............................................................................
