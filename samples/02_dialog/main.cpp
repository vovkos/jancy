#include "pch.h"
#include "MainWindow.h"

#if (_AXL_ENV == AXL_ENV_WIN)
#	pragma comment (linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

//.............................................................................

int main (int argc, char* argv [])
{
	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	err::registerParseErrorProvider ();

	srand ((int) axl::sys::getTimestamp ());

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
