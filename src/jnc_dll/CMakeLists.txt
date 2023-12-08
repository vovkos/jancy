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

set(CMAKE_INSTALL_RPATH)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

link_directories(
	${LLVM_LIB_DIR}
	${OPENSSL_LIB_DIR}
	${RE2S_LIB_DIR}
	${AXL_LIB_DIR}
)

if(WIN32)
	configure_file(jnc_dll.def.in jnc_dll.def)

	set(
		DLL_SRC_LIST
		dllmain.c
		jnc_dll.def.in
		${CMAKE_CURRENT_BINARY_DIR}/jnc_dll.def
	)
else()
	set(
		DLL_SRC_LIST
		$<TARGET_OBJECTS:jnc_api_core_o>
		$<TARGET_OBJECTS:jnc_rtl_core_o>
		$<TARGET_OBJECTS:jnc_rtl_intro_o>
		$<TARGET_OBJECTS:jnc_std_o>
		$<TARGET_OBJECTS:jnc_sys_o>
		jnc_dll.version
	)

	if(CMAKE_GENERATOR MATCHES "Xcode")
		set(
			DLL_SRC_LIST
			${DLL_SRC_LIST}
			xcode.cpp
		)
	endif()
endif()

source_group(
	dll
	FILES
	${DLL_SRC_LIST}
)

add_library(
	jnc_dll
	SHARED
	${DLL_SRC_LIST}
)

if(LINUX)
	set(AXL_CA_LIB_LIST axl_sys_ca axl_cry)
else()
	set(AXL_CA_LIB_LIST axl_sys_ca)
endif()

target_link_libraries(
	jnc_dll
	jnc_ct
	jnc_rt
	${AXL_CA_LIB_LIST}
	axl_zip
	axl_re2
	axl_io
	axl_lex
	axl_dox
	axl_core
	${RE2S_LIB_NAME}
)

if(WIN32)
	target_link_libraries(
		jnc_dll
		jnc_api_core
		jnc_rtl_core
		jnc_rtl_intro
		jnc_std
		jnc_sys
		wintrust
		crypt32
	)
else()
	target_link_libraries(
		jnc_dll
		pthread
	)

	if(APPLE)
		find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
		find_library(SECURITY_FRAMEWORK Security)

		target_link_libraries(
			jnc_dll
			axl_sec
			axl_cf
			${CORE_FOUNDATION_FRAMEWORK}
			${SECURITY_FRAMEWORK}
		)
	elseif(LINUX)
		target_link_libraries(
			jnc_dll
			${OPENSSL_CRYPTO_LIB_NAME}
			dl
			rt
		)

		set_property(
			TARGET jnc_dll
			APPEND_STRING
			PROPERTY LINK_FLAGS
			" -Wl,--version-script='${CMAKE_CURRENT_LIST_DIR}/jnc_dll.version'"
		)
	endif()
endif()

target_link_llvm_jit_libraries(jnc_dll)

set_target_properties(
	jnc_dll
	PROPERTIES
		LINKER_LANGUAGE CXX
		OUTPUT_NAME ${JANCY_DLL_NAME}
		VERSION ${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}.${JANCY_VERSION_REVISION}
		SOVERSION ${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_dll
	DESTINATION ${JANCY_INSTALL_DLL_SUBDIR}
)

#...............................................................................
