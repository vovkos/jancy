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

set(
	APP_H_LIST
	CmdLine.h
	JncApp.h
	version.h.in
)

set(
	APP_CPP_LIST
	main.cpp
	CmdLine.cpp
	JncApp.cpp
)

source_group(
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# res folder
#

set(
	RES_RC_LIST
	res.rc
)

source_group(
	res
	FILES
	${RES_RC_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

configure_file(
	version.h.in
	${GEN_DIR}/version.h
)

set(
	GEN_H_LIST
	${GEN_DIR}/version.h
)

axl_pop(CMAKE_CURRENT_BINARY_DIR)

source_group(
	gen
	FILES
	${GEN_H_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H   pch.h)

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_app jancy compiler
#

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
)

link_directories(
	${RE2S_LIB_DIR}
	${AXL_LIB_DIR}
)

if(BUILD_JNC_DLL)
	link_directories(
		${JANCY_LIB_DIR}
		${JANCY_BIN_DIR}
	)
else()
	link_directories(${LLVM_LIB_DIR})
endif()

add_executable(
	jnc_app
	${PCH_H}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
)

target_precompile_headers(
	jnc_app
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_app
	PROPERTIES
	OUTPUT_NAME jancy
	FOLDER tools
)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_app
		jnc_dll
	)

	target_link_libraries(
		jnc_app
		${JANCY_DLL_NAME}
	)
else()
	target_link_libraries(
		jnc_app
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		jnc_api_core
	)

	target_link_llvm_jit_libraries(jnc_app)
endif()

target_link_libraries(
	jnc_app
	axl_zip
	axl_re2
	axl_io
	axl_lex
	axl_core
	${RE2S_LIB_NAME}
)

if(WIN32)
	target_link_libraries(
		jnc_app
		ws2_32
	)
elseif(NOT APPLE)
	target_link_libraries(
		jnc_app
		pthread
		dl
		rt
	)
endif()

if(UNIX AND OPENSSL_DLL_DIR)
	# otherwise, SSL tests will fail if custom OpenSSL dylib-s are used

	set_target_properties(
		jnc_app
		PROPERTIES
		BUILD_RPATH ${OPENSSL_DLL_DIR}
	)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_app
	DESTINATION ${JANCY_INSTALL_BIN_SUBDIR}
)

#...............................................................................

axl_create_setting(
	JANCY_TEST_MODE
	DESCRIPTION "Compiler mode (run/JIT/compile/parse) when running tests"
	" "
	"--run"
	"--jit"
	"--compile-only"
	"--no-codegen"
)

if(UNIX)
	set(_DEBUG_INFO_DEFAULT "--debug")
else()
	set(_DEBUG_INFO_DEFAULT " ")
endif()

axl_create_setting(
	JANCY_TEST_DEBUG_INFO
	DESCRIPTION "Compile tests with debug information"
	DEFAULT ${_DEBUG_INFO_DEFAULT}
	" "
	"--debug"
)

axl_create_setting(
	JANCY_TEST_IR_OPT_LEVEL
	DESCRIPTION "LLVM IR optimization level to use when running tests"
	DEFAULT "-O2"
	"-O0"
	"-O1"
	"-O2"
	"-O3"
)

axl_create_setting(
	JANCY_TEST_JIT_OPT_LEVEL
	DESCRIPTION "JIT code generator optimization level to use when running tests"
	DEFAULT "-J2"
	"-J0"
	"-J1"
	"-J2"
	"-J3"
)

axl_create_setting(
	JANCY_TEST_JIT_KIND
	DESCRIPTION "JIT engine to use when running tests"
	DEFAULT " "
	" "
	"--mcjit"
	"--orc"
	"--legacy-jit"
)

macro(
add_jancy_tests
	# ...
)

	set(_ARG_LIST ${ARGN})

	set(
		_BASE_FLAG_LIST
		--import-dir ${JANCY_DLL_BASE_DIR}/$<CONFIGURATION>
		${JANCY_TEST_MODE}
		${JANCY_TEST_DEBUG_INFO}
		${JANCY_TEST_IR_OPT_LEVEL}
		${JANCY_TEST_JIT_OPT_LEVEL}
		${JANCY_TEST_JIT_KIND}
	)

	list(REMOVE_ITEM _BASE_FLAG_LIST " ")

	set(_COMMAND)
	set(_NAME_PREFIX)
	set(_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
	set(_FLAG_LIST)
	set(_REQUIRE_LIST)
	set(_FILE_LIST)

	foreach(_ARG ${_ARG_LIST})
		if("${_ARG}" MATCHES "NAME_PREFIX|WORKING_DIRECTORY|FLAGS|TESTS|REQUIRES|FILES|FINALIZE_TEST")
			set(_COMMAND ${_ARG})

			if (${_COMMAND} MATCHES "S$") # a list -- clear it
				string(REGEX REPLACE "S$" "_LIST" _LIST "${_COMMAND}")
				set(_${_LIST})
			endif()
		elseif("${_COMMAND}" STREQUAL "NAME_PREFIX")
			set(_NAME_PREFIX ${_ARG})
		elseif("${_COMMAND}" STREQUAL "WORKING_DIRECTORY")
			set(_WORKING_DIRECTORY ${_ARG})
		elseif("${_COMMAND}" STREQUAL "FLAGS")
			list(APPEND _FLAG_LIST ${_ARG})
		elseif("${_COMMAND}" STREQUAL "REQUIRES")
			list(APPEND _REQUIRE_LIST "--require" ${_ARG})
		elseif("${_COMMAND}" STREQUAL "FILES")
			list(APPEND _FILE_LIST ${_ARG})
		elseif("${_COMMAND}" STREQUAL "TESTS")
			add_test(
				NAME ${_NAME_PREFIX}${_ARG}
				WORKING_DIRECTORY ${_WORKING_DIRECTORY}
				COMMAND jnc_app ${_BASE_FLAG_LIST} ${_FLAG_LIST} ${_REQUIRE_LIST} ${_ARG}
			)
		elseif("${_COMMAND}" STREQUAL "FINALIZE_TEST")
			add_test(
				NAME ${_NAME_PREFIX}${_ARG}
				WORKING_DIRECTORY ${_WORKING_DIRECTORY}
				COMMAND jnc_app ${_BASE_FLAG_LIST} ${_FLAG_LIST} ${_REQUIRE_LIST} ${_FILE_LIST}
			)

			set(_COMMAND)
		else()
			message(FATAL_ERROR "syntax error in 'add_jancy_tests': ARG: '${_ARG}' COMMAND: '${_COMMAND}'")
		endif()
	endforeach()
endmacro()

#...............................................................................
