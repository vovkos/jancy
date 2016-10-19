# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#...............................................................................
#
# lib folder
#

set (
	LIB_H_LIST
	jnc_io_SocketAddress.h
	)

set (
	LIB_CPP_LIST
	jnc_io_SocketAddress.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_io_cmn library
#

include_directories (
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	)

add_library (
	jnc_io_cmn
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

axl_set_pch (
	jnc_io_cmn
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
