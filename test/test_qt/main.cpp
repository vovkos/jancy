#include "pch.h"
#include "mainwindow.h"

//.............................................................................

int main (int argc, char* argv [])
{
#if (_AXL_ENV == AXL_ENV_WIN)
	WSADATA WsaData;
	WSAStartup (0x0202, &WsaData);
#endif

	atexit (llvm::llvm_shutdown);

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	lex::registerParseErrorProvider ();
	srand ((int) sys::getTimestamp ());

	QApplication app (argc, argv);
	QCoreApplication::setOrganizationName ("Tibbo");
	QCoreApplication::setOrganizationDomain ("tibbo.com");
	QCoreApplication::setApplicationName ("JancyEdit");

	MainWindow mainWindow;
	mainWindow.showMaximized();

	return app.exec ();
}

//.............................................................................
