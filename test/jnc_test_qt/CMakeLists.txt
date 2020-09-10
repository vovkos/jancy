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

if(LINUX AND NOT ${TARGET_CPU} STREQUAL "x86") # if cross-compiling and64 -> x86, we gotta set rpath
	set(JNC_SKIP_BUILD_RPATH_DEFAULT TRUE)
endif()

option(JNC_SKIP_BUILD_RPATH "Skip build rpath (i.e. which QT to use)" ${JNC_SKIP_BUILD_RPATH_DEFAULT})

#...............................................................................
#
# app folder
#

set(
	APP_H_LIST
	monospaceplaintextedit.h
	llvmir.h
	llvmirhighlighter.h
	mainwindow.h
	mdichild.h
	modulepane.h
	output.h
	testlib.h
	)

set(
	APP_CPP_LIST
	monospaceplaintextedit.cpp
	llvmir.cpp
	llvmirhighlighter.cpp
	main.cpp
	mainwindow.cpp
	mdichild.cpp
	modulepane.cpp
	output.cpp
	testlib.cpp
	)

set(
	APP_RL_LIST
	llvmirhighlighter.rl
	)

source_group(
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

set(
	RES_PNG_LIST
	res/compile.png
	res/new.png
	res/open.png
	res/run.png
	res/save.png
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
	monospaceplaintextedit.h
	highlighter.h
	llvmir.h
	mainwindow.h
	mdichild.h
	modulepane.h
	output.h
	)

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

qt5_wrap_cpp(
	GEN_MOC_CPP_LIST
	${MOC_H_LIST}
	)

qt5_add_resources(
	GEN_QRC_CPP_LIST
	${RES_QRC_LIST}
	)

add_ragel_step(
	llvmirhighlighter.rl.cpp
	llvmirhighlighter.rl
	)

set(
	GEN_RL_CPP_LIST
	${GEN_DIR}/llvmirhighlighter.rl.cpp
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
# jnc_test_qt compiler test
#

if(JNC_SKIP_BUILD_RPATH)
	set(CMAKE_SKIP_BUILD_RPATH TRUE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'${JANCY_DLL_DIR}'")
endif()

if(BUILD_JNC_IO_USB)
	add_definitions(-D_JNC_IO_USB=1)
	include_directories(${LIBUSB_INC_DIR})
endif()

if(BUILD_JNC_IO_SSL)
	add_definitions(-D_JNC_IO_SSL=1)
	include_directories(${OPENSSL_INC_DIR})
endif()

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

if(BUILD_JNC_IO_USB)
	include_directories(${LIBUSB_INC_DIR})
	link_directories(${LIBUSB_LIB_DIR})
	add_definitions(-D_JNC_IO_USB=1)
endif()

if(BUILD_JNC_IO_SSL)
	include_directories(${OPENSSL_INC_DIR})
	link_directories(${OPENSSL_LIB_DIR})
	add_definitions(-D_JNC_IO_SSL=1)
endif()

add_definitions(
	-DQT_COMPILING_QSTRING_COMPAT_CPP
	-DQT_COMPILING_QIMAGE_COMPAT_CPP
	)

add_executable(
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

qt5_use_modules_alt(jnc_test_qt Widgets)

axl_set_pch(
	jnc_test_qt
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties(
	jnc_test_qt
	PROPERTIES
	FOLDER test
	)

target_link_libraries(
	jnc_test_qt
	jnc_edit
	)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_test_qt
		jnc_dll
		)

	target_link_libraries(
		jnc_test_qt
		${JANCY_DLL_NAME}
		)
else()
	target_link_libraries(
		jnc_test_qt
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		jnc_api_core
		)

	target_link_llvm_jit_libraries(jnc_test_qt)
endif()

if(BUILD_JNC_IO_USB)
	target_link_libraries(jnc_test_qt axl_io_usb)
endif()

if(BUILD_JNC_IO_SSL)
	target_link_libraries(
		jnc_test_qt
		axl_cry
		${OPENSSL_SSL_LIB_NAME}
		${OPENSSL_CRYPTO_LIB_NAME}
		)
endif()

target_link_libraries(
	jnc_test_qt
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	)

if(WIN32)
	target_link_libraries(
		jnc_test_qt
		ws2_32
		)
elseif(UNIX)
	target_link_libraries(
		jnc_test_qt
		pthread
		)

	if(NOT APPLE)
		target_link_libraries(
			jnc_test_qt
			rt
			)

		if(NOT JNC_SKIP_BUILD_RPATH)
			add_qt_rpath_link()
		endif()
	endif()
endif()

if(BUILD_JNC_IO_USB)
	if(WIN32)
		target_link_libraries(
			jnc_test_qt
			libusb-1.0
			)
	else()
		target_link_libraries(
			jnc_test_qt
			usb-1.0
			)

		if(APPLE)
			find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
			find_library(IOKIT_FRAMEWORK IOKit)

			target_link_libraries(
				jnc_test_qt
				${CORE_FOUNDATION_FRAMEWORK}
				${IOKIT_FRAMEWORK}
				)
		endif()

		if(LINUX)
			target_link_libraries(
				jnc_test_qt
				udev
				)
		endif()
	endif()
endif()

if(WIN32 AND QT_DLL_DIR)
	set(_DEBUG_SUFFIX $<$<CONFIG:Debug>:d>)

	add_custom_command(
		TARGET jnc_test_qt
		POST_BUILD
		COMMAND
			echo Copying DLL files for jnc_test_qt...
		COMMAND
			${CMAKE_COMMAND} -E copy
			${QT_DLL_DIR}/Qt5Core${_DEBUG_SUFFIX}.dll
			${QT_DLL_DIR}/Qt5Gui${_DEBUG_SUFFIX}.dll
			${QT_DLL_DIR}/Qt5Widgets${_DEBUG_SUFFIX}.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
		)
endif()

#...............................................................................
