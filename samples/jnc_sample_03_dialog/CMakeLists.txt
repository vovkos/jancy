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

set(
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
# gen folder
#

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

set(
	MOC_H_LIST
	QtSignalBridge.h
	MainWindow.h
)

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

qt5_wrap_cpp(
	GEN_MOC_CPP_LIST
	${MOC_H_LIST}
)

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

axl_exclude_from_build(
	${GEN_MOC_CPP_LIST}  # include "moc_*.cpp" manually
	${GEN_JNC_CPP_LIST}  # include "*.jnc.cpp" manually
)

source_group(
	gen
	FILES
	${GEN_MOC_CPP_LIST}
	${GEN_JNC_CPP_LIST}
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

#...............................................................................
#
# jnc_sample_03_dialog sample
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

add_definitions(
	-DQT_COMPILING_QSTRING_COMPAT_CPP
	-DQT_COMPILING_QIMAGE_COMPAT_CPP
)

add_executable(
	jnc_sample_03_dialog
	${PCH_H}
	${APP_H_LIST}
	${APP_CPP_LIST}
	${APP_JNC_LIST}
	${GEN_MOC_CPP_LIST}
	${GEN_JNC_CPP_LIST}
)

qt5_use_modules_alt(jnc_sample_03_dialog Widgets)

target_precompile_headers(
	jnc_sample_03_dialog
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_sample_03_dialog
	PROPERTIES
	FOLDER samples
)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_sample_03_dialog
		jnc_dll
	)

	target_link_libraries(
		jnc_sample_03_dialog
		${JANCY_DLL_NAME}
	)
else()
	target_link_libraries(
		jnc_sample_03_dialog
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

	target_link_llvm_jit_libraries(jnc_sample_03_dialog)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries(
		jnc_sample_03_dialog
		pthread
		dl
		rt
	)
endif()

if(WIN32 AND QT_DLL_DIR)
	set(_DEBUG_SUFFIX $<$<CONFIG:Debug>:d>)

	add_custom_command(
		TARGET jnc_sample_03_dialog
		POST_BUILD
		COMMAND
			echo Copying DLL files for jnc_sample_03_dialog...
		COMMAND
			${CMAKE_COMMAND} -E copy
			${QT_DLL_DIR}/Qt5Core${_DEBUG_SUFFIX}.dll
			${QT_DLL_DIR}/Qt5Gui${_DEBUG_SUFFIX}.dll
			${QT_DLL_DIR}/Qt5Widgets${_DEBUG_SUFFIX}.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
	)

	qt5_create_qt_conf(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
	qt5_create_qt_conf(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
endif()

# only add this test on Windows and Linux native build --
# it's a pain in the ass to install QT5 multilib on many Linux distros and
# on Mac it will not link to QT unless QT is properly placed in Frameworks folder

axl_detect_host_cpu(_CPU)

if(WIN32 OR(LINUX AND "${TARGET_CPU}" STREQUAL "${_CPU}"))
	add_test(
		NAME jnc_sample_03_dialog_default
		COMMAND $<TARGET_FILE:jnc_sample_03_dialog> --test
	)

	add_test(
		NAME jnc_sample_03_dialog
		COMMAND
			$<TARGET_FILE:jnc_sample_03_dialog>
			${CMAKE_CURRENT_LIST_DIR}/script.jnc
			--test
		)
endif()

#...............................................................................
