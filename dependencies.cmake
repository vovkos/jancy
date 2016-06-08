#..............................................................................

set (
	AXL_PATH_LIST

	LLVM_INC_DIR
	LLVM_LIB_DIR
	LLVM_CMAKE_DIR
	PCAP_INC_DIR
	PCAP_LIB_DIR
	LIBSSH2_INC_DIR
	LIBSSH2_LIB_DIR
	OPENSSL_INC_DIR
	OPENSSL_LIB_DIR
	QT_CMAKE_DIR
	QT_DLL_DIR
	AXL_CMAKE_DIR
	GRACO_CMAKE_DIR
	7Z_EXE
	PERL_EXE
	RAGEL_EXE
	DOXYGEN_EXE
	QDOC_EXE
	)

set (
	AXL_IMPORT_LIST
	
	REQUIRED 
		perl
		ragel
		llvm
		axl
		graco
		7z
	OPTIONAL 
		qt
		libssh2
		openssl
		pcap
	)

if (UNIX AND NOT APPLE)
	set (AXL_IMPORT_LIST ${AXL_IMPORT_LIST} OPTIONAL libudev)
endif ()

if (GRACO_CMAKE_DIR)
	set (_GRACO_CMAKE_DIR ${GRACO_CMAKE_DIR})
else ()
	set (_GRACO_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../graco/cmake)
endif ()

set (
	AXL_IMPORT_DIR_LIST
	
	${CMAKE_CURRENT_LIST_DIR}/cmake
	${_GRACO_CMAKE_DIR}
	)

#..............................................................................
