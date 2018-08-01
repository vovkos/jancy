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
# jnc_dll library
#

set (CMAKE_INSTALL_RPATH)
set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

link_directories (
	${LLVM_LIB_DIR}
	${AXL_LIB_DIR}
	)

if (BUILD_JNC_IO_USB)
	link_directories (${LIBUSB_LIB_DIR})
endif ()

if (WIN32)
	configure_file (jnc_dll.def.in jnc_dll.def)

	set (
		DLL_SRC_LIST
		dllmain.c
		jnc_dll.def.in
		${CMAKE_CURRENT_BINARY_DIR}/jnc_dll.def
		)
else ()
	set (
		DLL_SRC_LIST
		$<TARGET_OBJECTS:jnc_api_core_o>
		$<TARGET_OBJECTS:jnc_rtl_o>
		$<TARGET_OBJECTS:jnc_std_o>
		$<TARGET_OBJECTS:jnc_sys_o>
		jnc_dll.version
		)

	if (NOT APPLE)
		set (
			DLL_SRC_LIST
			${DLL_SRC_LIST}
			jnc_dll.version
			)
		endif ()
endif ()

source_group (
	dll
	FILES
	${DLL_SRC_LIST}
	)

add_library (
	jnc_dll SHARED
	${DLL_SRC_LIST}
	)

if (WIN32)
	target_link_libraries (
		jnc_dll
		jnc_api_core
		jnc_rtl
		jnc_std
		jnc_sys
		)
else ()
	target_link_libraries (
		jnc_dll
		pthread
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_dll
			rt
			)

		set_property (
			TARGET jnc_dll
			APPEND_STRING
			PROPERTY LINK_FLAGS
			" -Wl,--version-script='${CMAKE_CURRENT_LIST_DIR}/jnc_dll.version'"
			)
	endif ()
endif ()

target_link_libraries (
	jnc_dll
	jnc_ct
	jnc_rt
	axl_zip
	axl_fsm
	axl_io
	axl_lex
	axl_core
	)

target_link_llvm_jit_libraries (jnc_dll)

if (BUILD_JNC_IO_USB)
	target_link_libraries (
		jnc_dll
		${LIBUSB_LIB_NAME}
		)

	if (APPLE)
		find_library (CORE_FOUNDATION_FRAMEWORK CoreFoundation)
		find_library (IOKIT_FRAMEWORK IOKit)

		target_link_libraries (
			jnc_dll
			${CORE_FOUNDATION_FRAMEWORK}
			${IOKIT_FRAMEWORK}
			)
	endif ()

	if (LINUX)
		target_link_libraries (
			jnc_dll
			udev
			)
	endif ()
endif ()

set_target_properties (
	jnc_dll
	PROPERTIES
		LINKER_LANGUAGE CXX
		OUTPUT_NAME ${JANCY_DLL_NAME}
		VERSION ${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}.${JANCY_VERSION_REVISION}
		SOVERSION ${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install (
	TARGETS jnc_dll
	DESTINATION ${JANCY_INSTALL_DLL_SUBDIR}
	)

#...............................................................................
