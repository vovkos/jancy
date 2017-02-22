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
	MyLib.h
	TestClass.h
	TestStruct.h
	)

set (
	APP_C_LIST
	main.c
	MyLib.c
	TestClass.c
	TestStruct.c
	)

set (
	APP_JNC_LIST
	script.jnc
	)

source_group (
	app
	FILES
	${APP_H_LIST}
	${APP_C_LIST}
	${APP_JNC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H pch.h)
set (PCH_C pch.c)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_C}
	)

#...............................................................................
#
# jnc_sample_01_embed_c sample
#

include_directories (
	${JANCY_INC_DIR}
	)

link_directories (
	${AXL_LIB_DIR}
	${LLVM_LIB_DIR}
	)

add_executable (
	jnc_sample_01_embed_c
	${PCH_H}
	${PCH_C}
	${APP_H_LIST}
	${APP_C_LIST}
	${APP_JNC_LIST}
	)

set_target_properties (
	jnc_sample_01_embed_c
	PROPERTIES
	FOLDER samples
	)

if (FALSE)
	llvm_map_components_to_libraries (
		LLVM_LIB_LIST
		jit
		native
		MCJIT
		MCDisassembler
		)
else ()
	llvm_map_components_to_libnames (
		LLVM_LIB_LIST
		native
		MCJIT
		MCDisassembler
		)
endif ()

target_link_libraries (
	jnc_sample_01_embed_c
	jnc_api_static
	jnc_ct
	jnc_rt
	jnc_rtl
	jnc_std
	jnc_api_static
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	${LLVM_LIB_LIST}
	)

if (UNIX AND NOT APPLE)
	target_link_libraries (
		jnc_sample_01_embed_c
		rt
		)
endif ()

axl_set_pch (
	jnc_sample_01_embed_c
	${PCH_H}
	${PCH_C}
	)

add_test (
	NAME jnc_sample_01_embed_c
	COMMAND
		$<TARGET_FILE:jnc_sample_01_embed_c>
		${CMAKE_CURRENT_LIST_DIR}/script.jnc
	)

#...............................................................................
