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
	MyLib.h
	TestClass.h
	TestStruct.h
)

set(
	APP_CPP_LIST
	main.cpp
	MyLib.cpp
	TestClass.cpp
	TestStruct.cpp
)

set(
	APP_JNC_LIST
	script.jnc
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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step(
	script.jnc.cpp
	script.jnc
	${FILE2LITERAL_PL}
)

axl_pop(CMAKE_CURRENT_BINARY_DIR)

set(
	GEN_JNC_CPP_LIST
	${GEN_DIR}/script.jnc.cpp
)

axl_exclude_from_build(${GEN_JNC_CPP_LIST}) # include "*.jnc.cpp" manually

source_group(
	gen
	FILES
	${GEN_JNC_CPP_LIST}
)

#...............................................................................
#
# jnc_sample_02_export_cpp sample
#

include_directories(
	${JANCY_INC_DIR}
	${GEN_DIR}
)

if(BUILD_JNC_DLL)
	link_directories(
		${JANCY_LIB_DIR}
		${JANCY_BIN_DIR}
	)
else()
	link_directories(
		${LLVM_LIB_DIR}
		${AXL_LIB_DIR}
	)
endif()

if (GCC_LINK_GLIBC_WRAPPERS)
	link_directories(${AXL_LIB_DIR})
endif()

add_executable(
	jnc_sample_02_export_cpp
	${PCH_H}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	${GEN_JNC_CPP_LIST}
)

target_precompile_headers(
	jnc_sample_02_export_cpp
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_sample_02_export_cpp
	PROPERTIES
	FOLDER samples
)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_sample_02_export_cpp
		jnc_dll
	)

	target_link_libraries(
		jnc_sample_02_export_cpp
		${JANCY_DLL_NAME}
	)
else()
	target_link_libraries(
		jnc_sample_02_export_cpp
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		jnc_api_core
		axl_zip
		axl_re2
		axl_io
		axl_lex
		axl_core
	)

	target_link_llvm_jit_libraries(jnc_sample_02_export_cpp)
endif()

if(UNIX)
	target_link_libraries(
		jnc_sample_02_export_cpp
		pthread
	)

	if(NOT APPLE)
		target_link_libraries(
			jnc_sample_02_export_cpp
			rt
		)
	endif()
endif()

add_test(
	NAME jnc_sample_02_export_cpp_default
	COMMAND $<TARGET_FILE:jnc_sample_02_export_cpp>
)

add_test(
	NAME jnc_sample_02_export_cpp
	COMMAND
		$<TARGET_FILE:jnc_sample_02_export_cpp>
		${CMAKE_CURRENT_LIST_DIR}/script.jnc
	)

#...............................................................................
