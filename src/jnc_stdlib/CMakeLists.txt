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
# jnc_stdlib.jncx archive
#

set(DLL_DIR ${JANCY_DLL_BASE_DIR}/${CONFIGURATION})

add_custom_target(
	jnc_stdlib_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${DLL_DIR}/jnc_stdlib.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${DLL_DIR}/jnc_stdlib.jncx
		-x!jnc_StdTypes.jnc
		-x!jnc_StdFunctions.jnc
		${JANCY_ROOT_DIR}/src/jnc_ext/jnc_rtl/jnc/*
		${JANCY_ROOT_DIR}/src/jnc_ext/jnc_std/jnc/*
		${JANCY_ROOT_DIR}/src/jnc_ext/jnc_sys/jnc/*
	COMMAND ${7Z_EXE} a -tzip -y ${DLL_DIR}/jnc_stdlib.jncx
		${CMAKE_CURRENT_LIST_DIR}/.forced.jnc
	)

set_target_properties(
	jnc_stdlib_jncx
	PROPERTIES
	FOLDER extensions
	)

# no need to install (this is for IO Ninja/Jancy IDE only)

#...............................................................................
