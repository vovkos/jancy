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
# lib folder
#

set(
	LIB_H_LIST
	jnc_rtl_DynamicLayout.h
	jnc_rtl_DynamicLib.h
	jnc_rtl_Multicast.h
	jnc_rtl_Promise.h
	jnc_rtl_Reactor.h
	jnc_rtl_Regex.h
)

set(
	LIB_CPP_LIST
	jnc_rtl_CoreLib.cpp
	jnc_rtl_DynamicLayout.cpp
	jnc_rtl_DynamicLib.cpp
	jnc_rtl_Multicast.cpp
	jnc_rtl_Promise.cpp
	jnc_rtl_Reactor.cpp
	jnc_rtl_Reactor.cpp
	jnc_rtl_Regex.cpp
)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set(
	JNC_LIST
	jnc/jnc_gc.jnc
	jnc/jnc_DataPtr.jnc
	jnc/jnc_DynamicLayout.jnc
	jnc/jnc_DynamicLib.jnc
	jnc/jnc_Promise.jnc
	jnc/jnc_Regex.jnc
	jnc/jnc_Scheduler.jnc
	jnc/jnc_StdFunctions.jnc
	jnc/jnc_StdTypes.jnc
	jnc/jnc_StdTypedefs.jnc
)

source_group(
	jnc
	FILES
	${JNC_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H     pch.h)
set(PCH_O_CPP pch_o.cpp)

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

foreach(_JNC ${JNC_LIST})
	get_filename_component(_NAME ${_JNC} NAME)

	message("perl step: ${_JNC}")

	add_perl_step(
		${_NAME}.cpp
		${_JNC}
		${FILE2LITERAL_PL}
	)

	list(APPEND GEN_JNC_CPP_LIST ${GEN_DIR}/${_NAME}.cpp)
endforeach()

axl_pop(CMAKE_CURRENT_BINARY_DIR)

axl_exclude_from_build(${GEN_JNC_CPP_LIST})  # include "*.jnc.cpp" manually

source_group(
	gen
	FILES
	${GEN_JNC_CPP_LIST}
)

#...............................................................................
#
# jnc_rtl_core library
#

include_directories(
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_rt
	${JANCY_SRC_DIR}/jnc_ext/jnc_rtl_intro
	${GEN_DIR}
)

if(LINUX)
	include_directories(${OPENSSL_INC_DIR})
endif()

add_library(
	jnc_rtl_core
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	${GEN_JNC_CPP_LIST}
)

target_precompile_headers(
	jnc_rtl_core
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_rtl_core
	PROPERTIES
	FOLDER extensions
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# object library to be re-used as sources for the jancy shared/dylib

if(UNIX)
	add_library(
		jnc_rtl_core_o OBJECT
		${PCH_H}
		${PCH_O_CPP}
		${LIB_H_LIST}
		${LIB_CPP_LIST}
		${GEN_JNC_CPP_LIST}
	)

	target_compile_definitions(
		jnc_rtl_core_o
		PUBLIC
		-D_JNC_OBJECT_LIB=1
	)

	target_precompile_headers(
		jnc_rtl_core_o
		PRIVATE
	${PCH_H}
		${PCH_O_CPP}
	)

	set_target_properties(
		jnc_rtl_core_o
		PROPERTIES
		FOLDER extensions
	)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_rtl_core
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

#...............................................................................
