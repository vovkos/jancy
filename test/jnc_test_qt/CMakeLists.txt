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
	editor.h
	highlighter.h
	jancyhighlighter.h
	llvmir.h
	llvmirhighlighter.h
	mainwindow.h
	mdichild.h
	modulepane.h
	output.h
	testlib.h
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
	testlib.cpp
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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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
# jnc_test_qt compiler test
#

if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm200")
endif ()

include_directories (
	${PCAP_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	)

if (LIBUSB_FOUND)
	include_directories (${LIBUSB_INC_DIR})
	link_directories (${LIBUSB_LIB_DIR})

	add_definitions (-D_JNC_IO_USB=1)
endif ()

add_definitions (
	-DQT_COMPILING_QSTRING_COMPAT_CPP
	-DQT_COMPILING_QIMAGE_COMPAT_CPP
	)

link_directories (
	${AXL_LIB_DIR}
	${LLVM_LIB_DIR}
	${PCAP_LIB_DIR}
	)

add_definitions (-DQT_COMPILING_QSTRING_COMPAT_CPP)

add_executable (
	jnc_test_qt
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

qt5_use_modules (jnc_test_qt Widgets)

axl_set_pch (
	jnc_test_qt
	${PCH_H}
	${PCH_CPP}
	${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
	)

set_target_properties (
	jnc_test_qt
	PROPERTIES
	FOLDER test
	)

target_link_libraries (
	jnc_test_qt
	jnc_api_static
	jnc_ct
	jnc_rt
	jnc_rtl
	jnc_sys
	jnc_std
	jnc_api_static
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	)

target_link_llvm_jit_libraries (jnc_test_qt)

if (WIN32)
	target_link_libraries (
		jnc_test_qt
		ws2_32
		)
elseif (UNIX AND NOT APPLE)
	target_link_libraries (
		jnc_test_qt
		rt
		)
endif ()

if (LIBUSB_FOUND)
	if (WIN32)
		target_link_libraries (
			jnc_test_qt
			libusb-1.0
			)
	else ()
		target_link_libraries (
			jnc_test_qt
			usb-1.0
			)

		if (APPLE)
			find_library (CORE_FOUNDATION_FRAMEWORK CoreFoundation)
			find_library (IOKIT_FRAMEWORK IOKit)

			target_link_libraries (
				jnc_test_qt
				${CORE_FOUNDATION_FRAMEWORK}
				${IOKIT_FRAMEWORK}
				)
		endif ()

		if (LINUX)
			target_link_libraries (
				jnc_test_qt
				udev
				)
		endif ()
	endif ()
endif ()

if (WIN32)
	copy_qt_dll_files (jnc_test_qt Core Gui Widgets)
elseif (UNIX AND NOT APPLE)
	add_qt_rpath_link ()
endif ()

#...............................................................................
