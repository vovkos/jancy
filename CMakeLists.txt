# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

project (jancy)

cmake_minimum_required (VERSION 2.8)

include (${CMAKE_CURRENT_LIST_DIR}/../cmake/Master.cmake)

print_std_settings ()

check_and_print_std_libs (
	RAGEL
	LLVM
	QT
	AXL
	BULLDOZER
	)

if (QT_FOUND)
	unset (Qt5Core_DIR    CACHE)
	unset (Qt5Gui_DIR     CACHE)
	unset (Qt5Widgets_DIR CACHE)

	include ("${QT_CMAKE_DIR}/Qt5Core/Qt5CoreConfig.cmake")
	include ("${QT_CMAKE_DIR}/Qt5Widgets/Qt5WidgetsConfig.cmake")
endif ()

configure_file (
	cmake/jancy.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/cmake/jancy.cmake
	)

configure_file (
	cmake/jancy.cmake.in
	${CMAKE_CURRENT_LIST_DIR}/cmake/latest/jancy-${TARGET_CPU}.cmake
	)

configure_file (
	cmake/jancy-latest.cmake.in
	${CMAKE_CURRENT_LIST_DIR}/cmake/latest/jancy.cmake
	)

include (${LLVM_CMAKE_DIR}/LLVMConfig.cmake)
include (${AXL_CMAKE_DIR}/axl.cmake)
include (${BULLDOZER_CMAKE_DIR}/bulldozer.cmake)
include (${CMAKE_CURRENT_BINARY_DIR}/cmake/jancy.cmake)
include (version.cmake)

include_directories (${JANCY_INC_DIR})

add_subdirectory (
	samples
	${CMAKE_CURRENT_BINARY_DIR}/jnc_demos
	)

add_subdirectory (src)
add_subdirectory (tests)

#..............................................................................
