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
# lib folder
#

set(
	LIB_H_LIST
	jnc_rt_ExceptionMgr.h
	jnc_rt_Runtime.h
	jnc_rt_GcHeap.h
	jnc_rt_Event.h
)

set(
	LIB_CPP_LIST
	jnc_rt_ExceptionMgr.cpp
	jnc_rt_Runtime.cpp
	jnc_rt_GcHeap.cpp
)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H     pch.h)

source_group(
	pch
	FILES
	${PCH_H}
)

#...............................................................................
#
# jnc_rt library
#

include_directories(
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_ext/jnc_rtl_core
	${GEN_DIR}
)

if(LINUX)
	include_directories(${OPENSSL_INC_DIR})
endif()

add_library(
	jnc_rt
	${PCH_H}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
)

target_precompile_headers(
	jnc_rt
	PRIVATE
	${PCH_H}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_rt
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

#...............................................................................
