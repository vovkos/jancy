#include "pch.h"
#include "mainwindow.h"

int main (int argc, char* argv [])
{
	atexit (llvm::llvm_shutdown);

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	err::CParseErrorProvider::Register ();
	srand ((int) axl::g::GetTimestamp ());

	QApplication app (argc, argv);
	QCoreApplication::setOrganizationName ("Tibbo");
	QCoreApplication::setOrganizationDomain ("tibbo.com");
	QCoreApplication::setApplicationName ("JancyEdit");

	MainWindow mainWindow;
	mainWindow.showMaximized();

	return app.exec ();
}
