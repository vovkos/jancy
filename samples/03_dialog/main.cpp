#include "pch.h"
#include "MainWindow.h"

#if (_JNC_OS_WIN)
#	pragma comment (linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

//.............................................................................

int main (int argc, char* argv [])
{
	jnc::initialize ();

	QApplication app (argc, argv);
	QCoreApplication::setOrganizationName ("Tibbo");
	QCoreApplication::setOrganizationDomain ("tibbo.com");
	QCoreApplication::setApplicationName ("JancyDialog");

	MainWindow mainWindow;
	mainWindow.show ();
	mainWindow.runScript (argc >= 2 ? argv [1] : NULL);

	return app.exec ();
}

//.............................................................................
