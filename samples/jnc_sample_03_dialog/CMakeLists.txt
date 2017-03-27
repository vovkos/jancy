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
	MyWidget.h
	MyLayout.h
	MyLabel.h
	MyButton.h
	MyCheckBox.h
	MyTextEdit.h
	MySlider.h
	QtSignalBridge.h
	MainWindow.h
	)

set (
	APP_CPP_LIST
	MyLib.cpp
	MyWidget.cpp
	MyLayout.cpp
	MyLabel.cpp
	MyButton.cpp
	MyCheckBox.cpp
	MyTextEdit.cpp
	MySlider.cpp
	QtSignalBridge.cpp
	MainWindow.cpp
	main.cpp
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
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

set (
	MOC_H_LIST
	QtSignalBridge.h
	MainWindow.h
	)

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

qt5_wrap_cpp (
	GEN_MOC_CPP_LIST
	${MOC_H_LIST}
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

axl_exclude_from_build (${GEN_MOC_CPP_LIST}) # include "moc_*.cpp" manually

source_group (
	gen
	FILES
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
# jnc_sample_03_dialog sample
#

if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm200")
endif ()

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_INC_PRIVATE_DIR}
	${GEN_DIR}
	)

link_directories (
	${AXL_LIB_DIR}
	${LLVM_LIB_DIR}
	)

add_executable (
	jnc_sample_03_dialog
	${PCH_H}
	${PCH_CPP}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	${GEN_MOC_CPP_LIST}
	)

qt5_use_modules (jnc_sample_03_dialog Widgets)

axl_set_pch (
	jnc_sample_03_dialog
	${PCH_H}
	${PCH_CPP}
	${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
	)

set_target_properties (
	jnc_sample_03_dialog
	PROPERTIES
	FOLDER samples
	)

target_link_libraries (
	jnc_sample_03_dialog
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

target_link_llvm_jit_libraries (jnc_sample_03_dialog)

if (UNIX AND NOT APPLE)
	target_link_libraries (
		jnc_sample_03_dialog
		rt
		)
endif ()

if (WIN32)
	copy_qt_dll_files (jnc_sample_03_dialog Core Gui Widgets)
elseif (UNIX AND NOT APPLE)
	add_qt_rpath_link ()
endif ()

# only add this test on Windows and Linux native build --
# it's a pain in the ass to install QT5 multilib on many Linux distros and
# on Mac it will not link to QT unless QT is properly placed in Frameworks folder

axl_detect_host_cpu (_CPU)

if (WIN32 OR (LINUX AND "${TARGET_CPU}" STREQUAL "${_CPU}"))
	add_test (
		NAME jnc_sample_03_dialog
		COMMAND
			$<TARGET_FILE:jnc_sample_03_dialog>
			${CMAKE_CURRENT_LIST_DIR}/script.jnc
			--test
		)
endif ()

#...............................................................................
