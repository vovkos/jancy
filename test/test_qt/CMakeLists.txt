# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	editor.h
	highlighter.h
	jancyhighlighter.h
	llvmir.h
	llvmirhighlighter.h
	mainwindow.h
	mdichild.h
	modulepane.h
	output.h
	)

set (
	APP_CPP_LIST
	editor.cpp
	highlighter.cpp
	jancyhighlighter.cpp
	llvmir.cpp
	llvmirhighlighter.cpp
	main.cpp
	mainwindow.cpp
	mdichild.cpp
	modulepane.cpp
	output.cpp
	)

set (
	APP_RL_LIST
	jancyhighlighter.rl
	llvmirhighlighter.rl
	)

source_group (
	app
	FILES
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_RL_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# res folder
#

set (
	RES_PNG_LIST
	res/compile.png
	res/new.png
	res/open.png
	res/run.png
	res/save.png
	)

set (
	RES_QRC_LIST
	jancyedit.qrc
	)

source_group (
	res
	FILES
	${RES_PNG_LIST}
	${RES_QRC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

set (
	MOC_H_LIST
	editor.h
	highlighter.h
	jancyhighlighter.h
	llvmir.h
	llvmirhighlighter.h
	mainwindow.h
	mdichild.h
	modulepane.h
	output.h
	)

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

qt5_wrap_cpp (
	GEN_MOC_CPP_LIST
	${MOC_H_LIST}
	)

qt5_add_resources (
	GEN_QRC_CPP_LIST
	${RES_QRC_LIST}
	)

add_ragel_step (
	jancyhighlighter.rl.cpp
	jancyhighlighter.rl
	)

add_ragel_step (
	llvmirhighlighter.rl.cpp
	llvmirhighlighter.rl
	)

set (
	GEN_RL_CPP_LIST
	${GEN_DIR}/jancyhighlighter.rl.cpp
	${GEN_DIR}/llvmirhighlighter.rl.cpp
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

axl_exclude_from_build (${GEN_MOC_CPP_LIST}) # include "moc_*.cpp" manually
axl_exclude_from_build (${GEN_QRC_CPP_LIST}) # include "qrc_*.cpp" manually
axl_exclude_from_build (${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually

source_group (
	gen
	FILES
	${GEN_RL_CPP_LIST}
	${GEN_QRC_CPP_LIST}
	${GEN_MOC_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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

#..............................................................................
#
# test_qt compiler test
#

if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm200")
endif ()

include_directories (
	${LLVM_INC_DIR}
	${LLVM_INC_DIR_2}
	${PCAP_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

link_directories (
	${AXL_LIB_DIR}
	${LLVM_LIB_DIR}
	${PCAP_LIB_DIR}
	)

add_executable (
	test_qt
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_RL_LIST}
	${RES_PNG_LIST}
	${RES_QRC_LIST}
	${GEN_QRC_CPP_LIST}
	${GEN_MOC_CPP_LIST}
	${GEN_UI_H_LIST}
	${GEN_RL_CPP_LIST}
	)

llvm_map_components_to_libraries (
	LLVM_LIB_LIST
	jit
	native
	MCJIT
	MCDisassembler
	)

target_link_libraries (
	test_qt
	jnc_std
	jnc_core
	axl_zip
	axl_fsm
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

if (WIN32)
	target_link_libraries (
		test_qt
		ws2_32
		)
elseif (UNIX AND NOT APPLE)
	target_link_libraries (
		test_qt
		rt
		)
endif ()

qt5_use_modules (test_qt Widgets)

if (WIN32)
	copy_qt_dll_files (test_qt Core Gui Widgets)
elseif (UNIX AND NOT APPLE)
	add_qt_rpath_link ()
endif ()

axl_set_pch (
	test_qt
	${PCH_H}
	${PCH_CPP}
	${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
	)

set_target_properties (
	test_qt
	PROPERTIES
	FOLDER test
	)

#..............................................................................
