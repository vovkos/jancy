# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

project_virtual_folder (JncQtTest)

#..............................................................................
#
# app folder
#

set (
	APP_H_LIST
	disassembly.h
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
	disassembly.cpp
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

source_virtual_folder (
	app
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

source_virtual_folder (
	res
	${RES_PNG_LIST}
	${RES_QRC_LIST}
	)
 
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)

set (
	MOC_H_LIST
	disassembly.h
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

push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

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

pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_RL_CPP_LIST
	${GEN_DIR}/jancyhighlighter.rl.cpp
	${GEN_DIR}/llvmirhighlighter.rl.cpp
	)

exclude_from_build (${GEN_MOC_CPP_LIST}) # include "moc_*.cpp" manually
exclude_from_build (${GEN_QRC_CPP_LIST}) # include "qrc_*.cpp" manually
exclude_from_build (${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually

source_virtual_folder (
	gen
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

source_virtual_folder (
	pch
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
#
# JncQtTest compiler test
#

if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm200")
endif ()

include_directories (
	${PATH_AXL_INC_DIR}
	${PATH_LLVM_INC_DIR}
	${PATH_LLVM_INC_DIR_2}
	${GEN_DIR}
	)

link_directories (
	${PATH_AXL_LIB_DIR}
	${PATH_LLVM_LIB_DIR}
	)

add_executable (
	JncQtTest
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
	JncQtTest
	jnc
	axl_io
	axl_core
	${LLVM_LIB_LIST}
	)

qt5_use_modules (JncQtTest Widgets)

if (JNC_USE_PCH)
	set_pch (
		JncQtTest
		${PCH_H}
		${PCH_CPP}
		${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
		)
endif ()

set_target_properties (
	JncQtTest
	PROPERTIES
	FOLDER tests
	)

#..............................................................................
