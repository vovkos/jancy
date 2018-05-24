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

set (
	LIB_H_LIST
	jnc_io_AsyncIoDevice.h
	jnc_io_SocketAddress.h
	jnc_io_SocketOptions.h
	jnc_io_SocketBase.h
	)

set (
	LIB_CPP_LIST
	jnc_io_AsyncIoDevice.cpp
	jnc_io_SocketAddress.cpp
	jnc_io_SocketBase.cpp
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

target_compile_definitions (
	jnc_io_cmn
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB=1
	)

axl_set_pch (
	jnc_io_cmn
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties (
	jnc_io_cmn
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
