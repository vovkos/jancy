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
	test.h
)

set(
	APP_CPP_LIST
	main.cpp
	test.cpp
)

set(
	APP_JNC_LIST
	main.jnc
)

source_group(
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
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
# jnc_test_abi (ABI-compatibility test)
#

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
)

link_directories(${AXL_LIB_DIR})

if(BUILD_JNC_DLL)
	link_directories(
		${JANCY_LIB_DIR}
		${JANCY_BIN_DIR}
	)
else()
	link_directories(${LLVM_LIB_DIR})
endif()

add_executable(
	jnc_test_abi
	${PCH_H}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
)

target_precompile_headers(
	jnc_test_abi
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_test_abi
	PROPERTIES
	FOLDER test
)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_test_abi
		jnc_dll
	)

	target_link_libraries(
		jnc_test_abi
		${JANCY_DLL_NAME}
	)
else()
	target_link_libraries(
		jnc_test_abi
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		jnc_api_core
	)

	target_link_llvm_jit_libraries(jnc_test_abi)
endif()

target_link_libraries(
	jnc_test_abi
	axl_zip
	axl_io
	axl_lex
	axl_mem_new
	axl_core
)

if(WIN32)
	target_link_libraries(
		jnc_test_abi
		ws2_32
	)
elseif(UNIX)
	target_link_libraries(
		jnc_test_abi
		pthread
	)

	if(LINUX)
		target_link_libraries(
			jnc_test_abi
			dl
			rt
		)
	endif()
endif()

#...............................................................................

add_test(
	NAME "jnc-test-abi"
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	COMMAND jnc_test_abi main.jnc
)

#...............................................................................
