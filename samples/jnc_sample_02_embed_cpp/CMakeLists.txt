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
	APP_CPP_LIST
	main.cpp
	MyLib.cpp
	TestClass.cpp
	TestStruct.cpp
	)

set (
	APP_JNC_LIST
	script.jnc
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
# jnc_sample_02_embed_cpp sample
#

include_directories (
	${JANCY_INC_DIR}
	)

link_directories (
	${AXL_LIB_DIR}
	${LLVM_LIB_DIR}
	)

add_executable (
	jnc_sample_02_embed_cpp
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	)

axl_set_pch (
	jnc_sample_02_embed_cpp
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_sample_02_embed_cpp
	PROPERTIES
	FOLDER samples
	)

target_link_libraries (
	jnc_sample_02_embed_cpp
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
	)

target_link_llvm_jit_libraries (jnc_sample_02_embed_cpp)

if (UNIX AND NOT APPLE)
	target_link_libraries (
		jnc_sample_02_embed_cpp
		rt
		)
endif ()

add_test (
	NAME jnc_sample_02_embed_cpp
	COMMAND
		$<TARGET_FILE:jnc_sample_02_embed_cpp>
		${CMAKE_CURRENT_LIST_DIR}/script.jnc
	)

#...............................................................................
