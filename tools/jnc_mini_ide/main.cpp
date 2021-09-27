//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "mainwindow.h"

//..............................................................................

int main(int argc, char* argv[]) {
#if (_JNC_OS_WIN)
	WSADATA WsaData;
	WSAStartup(0x0202, &WsaData);
#endif

#if (_AXL_OS_POSIX)
	setvbuf(stdout, NULL, _IOLBF, 1024);
#endif

	g::getModule()->setTag("jnc_mini_ide");
	jnc::initialize("jnc_dll:jnc_mini_ide");
	jnc::setErrorRouter(err::getErrorMgr());
	lex::registerParseErrorProvider();
	sys::registerExceptionErrorProvider();

#if (_JNC_IO_USB)
	io::registerUsbErrorProvider();
#endif

#if (_JNC_IO_SSL)
	io::registerSslErrorProvider();
#endif

	uint_t seed = sys::getTimestamp();
	srand(seed);

	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("Tibbo");
	QCoreApplication::setOrganizationDomain("tibbo.com");
	QCoreApplication::setApplicationName("Jancy Mini-IDE");

	MainWindow mainWindow;
	mainWindow.showMaximized();

	return app.exec();
}

//..............................................................................
