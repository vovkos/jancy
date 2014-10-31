#include "pch.h"
#include "mainwindow.h"

int main (int argc, char* argv [])
{
	atexit (llvm::llvm_shutdown);

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	err::registerParseErrorProvider ();
	srand ((int) axl::g::getTimestamp ());

	QApplication app (argc, argv);
	QCoreApplication::setOrganizationName ("Tibbo");
	QCoreApplication::setOrganizationDomain ("tibbo.com");
	QCoreApplication::setApplicationName ("JancyEdit");

	MainWindow mainWindow;
	mainWindow.showMaximized();

	return app.exec ();
}
