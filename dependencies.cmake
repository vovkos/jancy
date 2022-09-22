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

set(
	AXL_PATH_LIST

	LLVM_INC_DIR
	LLVM_LIB_DIR
	LLVM_CMAKE_DIR
	PCAP_INC_DIR
	PCAP_LIB_DIR
	LIBSSH2_INC_DIR
	LIBSSH2_LIB_DIR
	LIBUSB_INC_DIR
	LIBUSB_LIB_DIR
	OPENSSL_INC_DIR
	OPENSSL_LIB_DIR
	QT_CMAKE_DIR
	QT_DLL_DIR
	AXL_CMAKE_DIR
	GRACO_CMAKE_DIR
	DOXYREST_CMAKE_DIR
	7Z_EXE
	PERL_EXE
	RAGEL_EXE
	DOXYGEN_EXE
	SPHINX_BUILD_EXE
	PDFLATEX_EXE
)

set(
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
		libusb
		devmon
		doxygen
		doxyrest
		sphinx
		latex
	)

if(UNIX AND NOT APPLE)
	set(AXL_IMPORT_LIST ${AXL_IMPORT_LIST} OPTIONAL libudev)
endif()

if (WIN32 AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(AXL_IMPORT_LIST ${AXL_IMPORT_LIST} REQUIRED nasm)
endif()

set(
	AXL_IMPORT_DIR_LIST

	${CMAKE_CURRENT_LIST_DIR}/cmake
	${GRACO_CMAKE_DIR}
	${DEVMON_CMAKE_DIR}
	${DOXYREST_CMAKE_DIR}
)

#...............................................................................
