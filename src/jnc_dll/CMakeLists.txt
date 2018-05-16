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

link_directories (
	${LLVM_LIB_DIR}
	${AXL_LIB_DIR}
	)

set (
	DLL_SRC_LIST
	$<TARGET_OBJECTS:jnc_api_static_o>
	$<TARGET_OBJECTS:jnc_rtl_o>
	$<TARGET_OBJECTS:jnc_std_o>
	$<TARGET_OBJECTS:jnc_sys_o>
	)

if (WIN32)
	configure_file (jnc_dll.def.in jnc_dll.def)

	set (
		DLL_SRC_LIST
		${DLL_SRC_LIST}
		dllmain.c
		jnc_dll.def.in
		${CMAKE_CURRENT_BINARY_DIR}/jnc_dll.def
		)
elseif (UNIX AND NOT APPLE)
	set (
		DLL_SRC_LIST
		${DLL_SRC_LIST}
		jnc_dll.version
		)
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

if (UNIX AND NOT APPLE)
	set_property (
		TARGET jnc_dll
		APPEND_STRING
		PROPERTY LINK_FLAGS
		" -Wl,--version-script='${CMAKE_CURRENT_LIST_DIR}/jnc_dll.version'"
		)
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

if (UNIX)
	target_link_libraries (
		jnc_dll
		pthread
		)

	if (NOT APPLE)
		target_link_libraries (
			jnc_dll
			rt
			)
	endif ()
endif ()

set_target_properties (
	jnc_dll
	PROPERTIES
		LINKER_LANGUAGE C
		OUTPUT_NAME jancy-${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install (
	TARGETS jnc_dll
	DESTINATION ${JANCY_INSTALL_DLL_SUBDIR}
	)

#...............................................................................
