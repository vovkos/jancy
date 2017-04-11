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

set (DOC_NAME "Jancy Grammar Reference")

set (RST_DIR  ${CMAKE_CURRENT_BINARY_DIR})
set (TMP_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tmp)
set (HTML_DIR ${JANCY_DOC_DIR}/html)

set (SPHINX_HTML_THEME ${AXL_SPHINX_HTML_THEME})

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (
	GRACO_CMD_LINE
	${JANCY_SRC_DIR}/jnc_ct/jnc_ct_Parser/jnc_ct_Parser.llk
	-b ${CMAKE_CURRENT_BINARY_DIR}/jancy.llk
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
	CMD_LINE_VAR_LIST

	GRACO_CMD_LINE
	SPHINX_HTML_CMD_LINE
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

configure_file (
	index.rst
	index.rst
	COPYONLY
	)

if (WIN32)
	string (REPLACE "$(Configuration)" "%1" GRACO_EXE ${GRACO_EXE})

	configure_file (
		build-llk.bat.in
		build-llk.bat
		)

	configure_file (
		${AXL_DOC_SHARE_DIR}/sh/build-html.bat.in
		build-html.bat
		)
else ()
	axl_configure_file_w_permissions (
		build-llk.sh.in
		build-llk.sh
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
endif ()

#...............................................................................
