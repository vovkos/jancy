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

#
# app folder
#

set (
	APP_H_LIST
	CmdLine.h
	JncApp.h
	version.h.in
	)

set (
	APP_CPP_LIST
	main.cpp
	CmdLine.cpp
	JncApp.cpp
	)

source_group (
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# res folder
#

set (
	RES_RC_LIST
	res.rc
	)

source_group (
	res
	FILES
	${RES_RC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

configure_file (
	version.h.in
	${GEN_DIR}/version.h
	)

set (
	GEN_H_LIST
	${GEN_DIR}/version.h
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

source_group (
	gen
	FILES
	${GEN_H_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H   pch.h)
set (PCH_CPP pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_app jancy compiler
#

include_directories (
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

link_directories (${AXL_LIB_DIR})

if (BUILD_JNC_DLL)
	link_directories (
		${JANCY_LIB_DIR}
		${JANCY_BIN_DIR}
		)
else ()
	link_directories (${LLVM_LIB_DIR})
endif ()

add_executable (
	jnc_app
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
	)

axl_set_pch (
	jnc_app
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_app
	PROPERTIES
	OUTPUT_NAME jancy
	)

if (BUILD_JNC_DLL)
	add_dependencies (
		jnc_app
		jnc_dll
		)

	target_link_libraries (
		jnc_app
		${JANCY_DLL_NAME}
		)
else ()
	target_link_libraries (
		jnc_app
		jnc_api_static
		jnc_ct
		jnc_rt
		jnc_rtl
		jnc_std
		jnc_sys
		jnc_api_static
		)

	target_link_llvm_jit_libraries (jnc_app)
endif ()

target_link_libraries (
	jnc_app
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	)

if (WIN32)
	target_link_libraries (
		jnc_app
		ws2_32
		)
elseif (UNIX)
	target_link_libraries (
		jnc_app
		pthread
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_app
			rt
			)
	endif ()
endif ()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install (
	TARGETS jnc_app
	DESTINATION ${JANCY_INSTALL_BIN_SUBDIR}
	)

#...............................................................................

macro (
add_jancy_tests
	# ...
	)

	set (_FLAGS "--print-retval")

	if (UNIX)
		set (_FLAGS "${_FLAGS} --debug-info")
	endif ()

	string (REPLACE " " ";" _FLAG_LIST "${_FLAGS}")

	set (_ARG_LIST ${ARGN})
	set (_DIR)
	set (_PREFIX)
	set (_FILE_LIST)
	set (_FIRST_FILE)
	set (_IS_DIR FALSE)
	set (_IS_PREFIX FALSE)
	set (_IS_FLAGS FALSE)
	set (_IS_FILE_LIST FALSE)

	foreach (_ARG ${_ARG_LIST})
		if (_IS_DIR)
			set (_DIR ${_ARG})
			set (_IS_DIR FALSE)
		elseif (_IS_PREFIX)
			set (_PREFIX ${_ARG})
			set (_IS_PREFIX FALSE)
		elseif (_IS_FLAGS)
			set (_FLAGS "${_FLAGS} ${_ARG}")
			set (_IS_FLAGS FALSE)
			string (REPLACE " " ";" _FLAG_LIST "${_FLAGS}")
		elseif ("${_ARG}" STREQUAL "WORKING_DIRECTORY")
			set (_IS_DIR TRUE)
		elseif ("${_ARG}" STREQUAL "NAME_PREFIX")
			set (_IS_PREFIX TRUE)
		elseif ("${_ARG}" STREQUAL "FLAGS")
			set (_IS_FLAGS TRUE)
		elseif ("${_ARG}" STREQUAL "BEGIN_FILE_LIST")
			set (_IS_FILE_LIST TRUE)
			set (_FILE_LIST)
		elseif (_IS_FILE_LIST AND NOT "${_ARG}" STREQUAL "END_FILE_LIST")
			if ("${_FILE_LIST}" STREQUAL "")
				set (_FIRST_FILE ${_ARG})
			endif ()

			list (APPEND _FILE_LIST ${_ARG})
		else ()
			if (_IS_FILE_LIST)
				set (_IS_FILE_LIST FALSE)
			else ()
				set (_FIRST_FILE ${_ARG})
				set (_FILE_LIST ${_ARG})
			endif ()

			if ("${_DIR}" STREQUAL "")
				add_test (
					NAME ${_PREFIX}${_FIRST_FILE}
					COMMAND jnc_app ${_FLAG_LIST} ${_FILE_LIST}
					)
			else ()
				add_test (
					NAME ${_PREFIX}${_FIRST_FILE}
					WORKING_DIRECTORY ${_DIR}
					COMMAND jnc_app ${_FLAG_LIST} ${_FILE_LIST}
					)
			endif ()
		endif ()
	endforeach ()
endmacro ()

#...............................................................................
