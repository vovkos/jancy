# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

set (VIRTUAL_FOLDER_SUB_DIR JncMfcTest)

#..............................................................................
#
# src folder
#

set (
	SRC_H_LIST
	AstDoc.h
	AstPane.h
	DasmPane.h
	LlvmIrPane.h
	LogCtrl.h
	MainFrm.h
	ModulePane.h
	OutputPane.h
	Resource.h
	test_ast.h
	)

set (
	SRC_CPP_LIST
	AstDoc.cpp
	AstPane.cpp
	DasmPane.cpp
	LlvmIrPane.cpp
	LogCtrl.cpp
	MainFrm.cpp
	ModulePane.cpp
	OutputPane.cpp
	test_ast.cpp
	)

virtual_folder (
	src
	${SRC_H_LIST}
	${SRC_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# res folder
#

set (
	RES_BMP_LIST
	res/buttons.bmp
	res/filelarge.bmp
	res/filesmall.bmp
	res/info.bmp
	res/main.bmp
	res/nav_large.bmp
	res/nav_large_hc.bmp
	res/pages.bmp
	res/pages_hc.bmp
	res/pages_small.bmp
	res/pages_small_hc.bmp
	res/Toolbar.bmp
	res/Toolbar256.bmp
	res/writelarge.bmp
	res/writesmall.bmp
	)

set (
	RES_ICO_LIST
	res/test_ast.ico
	res/test_astDoc.ico
	)

set (
	RES_XML_LIST
	res/ribbon.mfcribbon-ms
	)

set (
	RES_RC_LIST
	test_ast.rc
	res/test_ast.rc2
	)

virtual_folder (
	res
	${RES_BMP_LIST}
	${RES_ICO_LIST}
	${RES_XML_LIST}
	${RES_RC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H   pch.h)
set (PCH_CPP pch.cpp)

virtual_folder (
	pch
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
#
# JncMfcTest compiler test
#

include_directories (
	${PATH_LLVM_INC_DIR}
	${PATH_LLVM_INC_DIR_2}
	)

link_directories (
	${PATH_LLVM_LIB_DIR}/$(Configuation)
	)

set (CMAKE_MFC_FLAG 1)

add_executable (
	JncMfcTest WIN32
	${PCH_H}
	${PCH_CPP}
	${SRC_H_LIST}
	${SRC_CPP_LIST}
	${RES_BMP_LIST}
	${RES_ICO_LIST}
	${RES_XML_LIST}
	${RES_RC_LIST}
	)

llvm_map_components_to_libraries (
	LLVM_LIB_LIST
	jit
	native
	MCDisassembler
	)

target_link_libraries (
	JncMfcTest
	axl_core
	axl_io
	axl_jnc
	axl_parse
	${LLVM_LIB_LIST}
	)

set_pch (
	JncMfcTest
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	JncMfcTest
	PROPERTIES
	FOLDER tests
	LINKER_LANGUAGE C
	)

#..............................................................................
