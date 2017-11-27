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

int exec (QApplication* app)
{
	int result = 0;

	JNC_BEGIN_SEH_FRAME ()
	result = app->exec ();
	JNC_END_SEH_FRAME ()

	return result;
}

int main (int argc, char* argv [])
{
#if (_JNC_OS_WIN)
	WSADATA WsaData;
	WSAStartup (0x0202, &WsaData);
#endif

#if (_AXL_OS_POSIX)
	setvbuf (stdout, NULL, _IOLBF, 1024);
#endif

	jnc::initialize ("jnc_test_qt");

#if (_JNC_IO_USB)
	io::registerUsbErrorProvider ();
#endif

	srand ((int) sys::getTimestamp ());

	QApplication app (argc, argv);
	QCoreApplication::setOrganizationName ("Tibbo");
	QCoreApplication::setOrganizationDomain ("tibbo.com");
	QCoreApplication::setApplicationName ("JancyEdit");

	MainWindow mainWindow;
	mainWindow.showMaximized();

	return exec (&app);
}

//..............................................................................
