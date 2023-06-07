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

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_mini_ide - Jancy Minimalistic IDE
#

if(BUILD_JNC_IO_USB)
#	add_definitions(-D_JNC_IO_USB=1)
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
	jnc_mini_ide
	${PCH_H}
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

qt5_use_modules_alt(jnc_mini_ide Widgets)

target_precompile_headers(
	jnc_mini_ide
	PRIVATE
	${PCH_H}
)

set_target_properties(
	jnc_mini_ide
	PROPERTIES
	FOLDER tools
)

target_link_libraries(
	jnc_mini_ide
	jnc_edit
)

if(BUILD_JNC_DLL)
	add_dependencies(
		jnc_mini_ide
		jnc_dll
	)

	target_link_libraries(
		jnc_mini_ide
		${JANCY_DLL_NAME}
	)
else()
	target_link_libraries(
		jnc_mini_ide
		jnc_api_core
		jnc_ct
		jnc_rt
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		jnc_api_core
	)

	target_link_llvm_jit_libraries(jnc_mini_ide)
endif()

if(BUILD_JNC_IO_USB)
	target_link_libraries(jnc_mini_ide axl_io_usb_error)
endif()

if(BUILD_JNC_IO_SSL)
	target_link_libraries(
		jnc_mini_ide
		axl_cry
		${OPENSSL_SSL_LIB_NAME}
		${OPENSSL_CRYPTO_LIB_NAME}
	)
endif()

target_link_libraries(
	jnc_mini_ide
	axl_zip
	axl_re
	axl_io
	axl_lex
	axl_core
)

if(WIN32)
	target_link_libraries(
		jnc_mini_ide
		ws2_32
	)
elseif(UNIX)
	target_link_libraries(
		jnc_mini_ide
		pthread
	)

	if(LINUX)
		target_link_libraries(
			jnc_mini_ide
			dl
			rt
		)
	endif()
endif()

if(WIN32 AND QT_DLL_DIR)
	set(_DEBUG_SUFFIX $<$<CONFIG:Debug>:d>)

	add_custom_command(
		TARGET jnc_mini_ide
		POST_BUILD
		COMMAND
			echo Copying DLL files for jnc_mini_ide...
		COMMAND
			${CMAKE_COMMAND} -E copy
			${QT_DLL_DIR}/Qt5Core${_DEBUG_SUFFIX}.dll
			${QT_DLL_DIR}/Qt5Gui${_DEBUG_SUFFIX}.dll
			${QT_DLL_DIR}/Qt5Widgets${_DEBUG_SUFFIX}.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIGURATION}
		)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_mini_ide
	DESTINATION ${JANCY_INSTALL_BIN_SUBDIR}
)

#...............................................................................
