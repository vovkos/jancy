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
	LIB_H_LIST
	${JANCY_INC_DIR}/jnc_Edit.h
	jnc_Edit_p.h
	jnc_LineNumberMargin.h
	jnc_Highlighter.h
	)

set(
	LIB_CPP_LIST
	jnc_Edit_p.cpp
	jnc_LineNumberMargin.cpp
	jnc_Highlighter.cpp
	)

set(
	LIB_RL_LIST
	jnc_Highlighter.rl
	)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_RL_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# res folder
#

set(
	RES_PNG_LIST
	res/objects.png
	)

set(
	RES_QRC_LIST
	res.qrc
	)

source_group(
	res
	FILES
	${RES_PNG_LIST}
	${RES_QRC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

set(
	MOC_H_LIST
	${JANCY_INC_DIR}/jnc_Edit.h
	jnc_Edit_p.h
	jnc_LineNumberMargin.h
	)

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

qt5_wrap_cpp_alt(
	GEN_MOC_CPP_LIST
	${MOC_H_LIST}
	)

qt5_add_resources(
	GEN_QRC_CPP_LIST
	${RES_QRC_LIST}
	)

add_ragel_step(
	jnc_Highlighter.rl.cpp
	jnc_Highlighter.rl
	)

set(
	GEN_RL_CPP_LIST
	${GEN_DIR}/jnc_Highlighter.rl.cpp
	)

axl_pop(CMAKE_CURRENT_BINARY_DIR)

axl_exclude_from_build(${GEN_MOC_CPP_LIST}) # include "moc_*.cpp" manually
axl_exclude_from_build(${GEN_QRC_CPP_LIST}) # include "qrc_*.cpp" manually
axl_exclude_from_build(${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually

source_group(
	gen
	FILES
	${GEN_RL_CPP_LIST}
	${GEN_QRC_CPP_LIST}
	${GEN_MOC_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H   pch.h)
set(PCH_CPP pch.cpp)

source_group(
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_edit compiler test
#

include_directories(
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
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

add_definitions(
	-DQT_COMPILING_QSTRING_COMPAT_CPP
	-DQT_COMPILING_QIMAGE_COMPAT_CPP
	-D_JNC_EDIT_DLL=1
	)

add_library(
	jnc_edit SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${LIB_RL_LIST}
	${RES_PNG_LIST}
	${RES_QRC_LIST}
	${GEN_QRC_CPP_LIST}
	${GEN_MOC_CPP_LIST}
	${GEN_RL_CPP_LIST}
	)

qt5_use_modules(jnc_edit Widgets)

axl_set_pch(
	jnc_edit
	${PCH_H}
	${PCH_CPP}
	${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
	)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_edit
		jnc_dll
		)

	target_link_libraries(
		jnc_edit
		${JANCY_DLL_NAME}
		)
else()
	target_link_libraries(
		jnc_edit
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		jnc_api_core
		)

	target_link_llvm_jit_libraries(jnc_edit)
endif()

target_link_libraries(
	jnc_edit
	axl_zip
	axl_io
	axl_lex
	axl_core
	)

if(WIN32)
	target_link_libraries(
		jnc_edit
		ws2_32
		)
elseif(UNIX)
	target_link_libraries(
		jnc_edit
		pthread
		)

	if(NOT APPLE)
		target_link_libraries(
			jnc_edit
			rt
			)

		add_qt_rpath_link()
	endif()
endif()

set_target_properties(
	jnc_edit
	PROPERTIES
		OUTPUT_NAME ${JANCY_EDIT_DLL_NAME}
		VERSION ${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}.${JANCY_VERSION_REVISION}
		SOVERSION ${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}
	)

#...............................................................................
