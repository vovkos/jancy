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
#include "MainWindow.h"

#if (_JNC_OS_WIN)
#	pragma comment (linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

//..............................................................................

int main (int argc, char* argv [])
{
	jnc::initialize ();

	bool isTest = false;
	QString fileName;
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv [i];

		if (arg [0] != '-')
			fileName = arg;
		else if (strcmp (arg, "--test") == 0)
			isTest = true;
	}

	QApplication app (argc, argv);
	QCoreApplication::setOrganizationName ("Tibbo");
	QCoreApplication::setOrganizationDomain ("tibbo.com");
	QCoreApplication::setApplicationName ("JancyDialog");

	MainWindow mainWindow;
	mainWindow.show ();
	bool result = mainWindow.runScript (fileName);

	if (isTest)
	{
		printf ("%s\n", mainWindow.readOutput ().toUtf8 ().constData ());
		return result ? 0 : -1;
	}

	return app.exec ();
}

//..............................................................................
