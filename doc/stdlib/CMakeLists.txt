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

set (DOC_NAME "Jancy Standard Library Reference")

set (TEX_BASE_NAME jancy-stdlib-${JANCY_VERSION_FILE_SUFFIX})
set (TEX_FILE_NAME ${TEX_BASE_NAME}.tex)
set (PDF_FILE_NAME ${TEX_BASE_NAME}.pdf)

set (STATIC_RST_DIR ${CMAKE_CURRENT_LIST_DIR}/rst)

set (XML_DIR  ${CMAKE_CURRENT_BINARY_DIR}/xml)
set (RST_DIR  ${CMAKE_CURRENT_BINARY_DIR}/rst)
set (HTML_DIR ${CMAKE_CURRENT_BINARY_DIR}/html)
set (TEX_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tex)
set (PDF_DIR  ${CMAKE_CURRENT_BINARY_DIR}/pdf)
set (TMP_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tmp)

set (SPHINX_HTML_THEME ${AXL_SPHINX_HTML_THEME})

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (
	JANCY_CMD_LINE
	-d
	--std-lib-doc
	--no-doxy-comment ///
	-O ${XML_DIR}
	${JANCY_SRC_DIR}/jnc_ct/jnc/jnc_StdTypedefs.jnc
	${JANCY_SRC_DIR}/jnc_ct/jnc/jnc_DynamicLib.jnc
	${JANCY_SRC_DIR}/jnc_ct/jnc/jnc_Recognizer.jnc
	${JANCY_SRC_DIR}/jnc_ct/jnc/jnc_Scheduler.jnc
	${JANCY_SRC_DIR}/jnc_ext/jnc_std/jnc/std_globals.jnc
	${JANCY_SRC_DIR}/jnc_ext/jnc_std/jnc/std_Error.jnc
	${JANCY_SRC_DIR}/jnc_ext/jnc_std/jnc/std_List.jnc
	${JANCY_SRC_DIR}/jnc_ext/jnc_std/jnc/std_HashTable.jnc
	-S ${JANCY_SRC_DIR}/jnc_ext/jnc_sys/jnc
	-I ${JANCY_SRC_DIR}/jnc_ext/jnc_io_base/jnc
	-S ${JANCY_SRC_DIR}/jnc_ext/jnc_io_base/jnc
	-S ${JANCY_SRC_DIR}/jnc_ext/jnc_io_ssh/jnc
	-S ${JANCY_SRC_DIR}/jnc_ext/jnc_io_pcap/jnc
	-S ${JANCY_SRC_DIR}/jnc_ext/jnc_io_usb/jnc
	-S ${CMAKE_CURRENT_LIST_DIR}/dox
	)

set (XMLGEN_EXE ${JANCY_EXE})
set (XMLGEN_CMD_LINE ${JANCY_CMD_LINE})

set (
	DOXYREST_CMD_LINE
	${XML_DIR}/index.xml
	-o ${RST_DIR}/global.rst
	-F ${DOXYREST_FRAME_DIR}
	-D g_language=jancy
	-D g_globalDefFile=def.rst
	-f c_global.rst.in
	)

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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (
	CMD_LINE_VAR_LIST

	XMLGEN_CMD_LINE
	DOXYREST_CMD_LINE
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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

configure_file (
	conf.py.in
	conf.py
	)

if (WIN32)
	string (REPLACE "$(Configuration)" "%1" XMLGEN_EXE ${XMLGEN_EXE})
	string (REPLACE "$(Configuration)" "%1" DOXYREST_EXE ${DOXYREST_EXE})

	configure_file (
		${AXL_DOC_SHARE_DIR}/sh/build-xml.bat.in
		build-xml.bat
		)

	configure_file (
		${AXL_DOC_SHARE_DIR}/sh/build-rst.bat.in
		build-rst.bat
		)

	configure_file (
		${AXL_DOC_SHARE_DIR}/sh/build-html.bat.in
		build-html.bat
		)

	configure_file (
		${AXL_DOC_SHARE_DIR}/sh/build-pdf.bat.in
		build-pdf.bat
		)
else ()
	axl_configure_file_w_permissions (
		${AXL_DOC_SHARE_DIR}/sh/build-xml.sh.in
		build-xml.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions (
		${AXL_DOC_SHARE_DIR}/sh/build-rst.sh.in
		build-rst.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions (
		${AXL_DOC_SHARE_DIR}/sh/build-html.sh.in
		build-html.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions (
		${AXL_DOC_SHARE_DIR}/sh/build-pdf.sh.in
		build-pdf.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif ()

#...............................................................................
