#include "pch.h"
#include "MainWindow.h"
#include "moc_MainWindow.cpp"
#include "MyLib.h"

//.............................................................................

static MainWindow* g_mainWindow = NULL;

MainWindow* getMainWindow ()
{
	return g_mainWindow;
}

//.............................................................................

MainWindow::MainWindow (
	QWidget *parent, 
	Qt::WindowFlags flags
	): 
	QMainWindow (parent, flags)
{
	ASSERT (!g_mainWindow);
	g_mainWindow = this;

	m_body = new QWidget (this);
	m_body->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	setCentralWidget (m_body);

	QFont font ("Monospace", 9);
	font.setStyleHint (QFont::TypeWriter);

	m_output = new QPlainTextEdit (this);
	m_output->setReadOnly(true);
	m_output->setTextInteractionFlags (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_output->setLineWrapMode (QPlainTextEdit::NoWrap);
	m_output->setFont (font);

	QDockWidget* dockWidget = new QDockWidget ("Output", this);
	dockWidget->setWidget (m_output);
	addDockWidget (Qt::BottomDockWidgetArea, dockWidget);

	mt::setTlsSlotValue (&m_module);
	mt::setTlsSlotValue (&m_runtime);
}

int MainWindow::output_va (const char* format, va_list va)
{
	QString string;
	string.vsprintf (format, va);
	m_output->moveCursor (QTextCursor::End);
	m_output->insertPlainText (string);
	return string.length ();
}

bool MainWindow::runScript (const QString& fileName_qt)
{
	if (fileName_qt.isEmpty ())
	{
		output ("usage: 02_dialog <script.jnc>\n");
		return false;
	}

	sl::String fileName = (const utf16_t*) fileName_qt.unicode ();

	output ("Opening '%s'...\n", fileName.cc ());

	io::SimpleMappedFile file;
	bool result = file.open (fileName, io::FileFlag_ReadOnly);
	if (!result)
	{
		output ("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	output ("Parsing...\n");

	jnc::ext::ExtensionLibHost* libHost = jnc::ext::getStdExtensionLibHost ();

	result = 
		m_module.create (fileName) &&
		m_module.m_extensionLibMgr.addLib (jnc::ext::getStdLib (libHost));
		m_module.m_extensionLibMgr.addLib (getMyLib (libHost));

	result = m_module.parse (
		fileName,
		(const char*) file.p (),
		file.getSize ()
		);

	if (!result)
	{
		output ("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	output ("Compiling...\n");

	result = m_module.compile ();
	if (!result)
	{
		output ("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	output ("JITting...\n");

	result = 
		m_module.createLlvmExecutionEngine () &&
		m_module.jit ();

	if (!result)
	{
		output ("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	jnc::ct::Namespace* nspace = m_module.m_namespaceMgr.getGlobalNamespace ();
	jnc::ct::Function* mainFunction = nspace->getFunctionByName ("main");
	if (!mainFunction)
	{
		output ("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	output ("Running...\n");

	result = m_runtime.startup (&m_module);
	if (!result)
	{
		output ("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	m_layout = jnc::rt::createClass <MyLayout> (&m_runtime, QBoxLayout::TopToBottom);
	m_body->setLayout (m_layout->m_qtLayout);

	int returnValue;
	result = jnc::rt::callFunction (&m_runtime, mainFunction, &returnValue, (jnc::rt::IfaceHdr*) &m_layout);
	if (!result)
	{
		output ("Runtime error: %s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	output ("Done.\n");
	return true;
}

void MainWindow::closeEvent (QCloseEvent* e)
{
	output ("Shutting down...\n");
	m_runtime.shutdown ();
}

//.............................................................................
