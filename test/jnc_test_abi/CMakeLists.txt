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
	test.h
	)

set (
	APP_CPP_LIST
	main.cpp
	test.cpp
	)

set (
	APP_JNC_LIST
	main.jnc
	)

source_group (
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
# jnc_test_abi (ABI-compatibility test)
#

include_directories (
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
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
	jnc_test_abi
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	${RES_RC_LIST}
	${GEN_H_LIST}
	)

axl_set_pch (
	jnc_test_abi
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_test_abi
	PROPERTIES
	OUTPUT_NAME jancy
	)

if (BUILD_JNC_DLL)
	add_dependencies (
		jnc_test_abi
		jnc_dll
		)

	target_link_libraries (
		jnc_test_abi
		${JANCY_DLL_NAME}
		)
else ()
	target_link_libraries (
		jnc_test_abi
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl
		jnc_std
		jnc_sys
		jnc_api_core
		)

	target_link_llvm_jit_libraries (jnc_test_abi)
endif ()

target_link_libraries (
	jnc_test_abi
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	)

if (WIN32)
	target_link_libraries (
		jnc_test_abi
		ws2_32
		)
elseif (UNIX)
	target_link_libraries (
		jnc_test_abi
		pthread
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_test_abi
			rt
			)
	endif ()
endif ()

#...............................................................................
