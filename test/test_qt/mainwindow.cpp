#include "pch.h"
#include "mainwindow.h"
#include "mdichild.h"
#include "output.h"
#include "modulepane.h"
#include "llvmir.h"
#include "disassembly.h"
#include "moc_mainwindow.cpp"
#include "qrc_jancyedit.cpp"

#include "axl_g_WarningSuppression.h"

//.............................................................................

void
TestClass::enumGcRoots (
	jnc::Runtime* runtime,
	jnc::IfaceHdr* iface
	)
{
	TestClass* self = (TestClass*) iface;
	if (self->m_hiddenIface)
		self->m_hiddenIface->m_object->gcMarkObject (runtime);
}

TestClass*
TestClass::operatorNew ()
{
	jnc::ApiObjBox <TestClass>* test = (jnc::ApiObjBox <TestClass>*) jnc::StdLib::gcAllocate (getApiType (), 1);
	test->prime ();
	return test;
}

void
AXL_CDECL
TestClass::destruct ()
{
	printf ("TestClass::destruct ()\n");
}

void
AXL_CDECL
TestClass::foo (jnc::IfaceHdr* iface)
{
	printf ("TestClass::foo (iface = %x)\n", iface);
	m_hiddenIface = iface;
}

//.............................................................................

void
AXL_CDECL
TestStruct::construct_0 (jnc::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::construct_1 (jnc::DataPtr selfPtr, int x)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
	self->m_x = x;
}

void
AXL_CDECL
TestStruct::construct_2 (jnc::DataPtr selfPtr, double y)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
	self->m_y = y;
}

void
AXL_CDECL
TestStruct::foo_0 (jnc::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::foo_1 (jnc::DataPtr selfPtr, int x)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::foo_2 (jnc::DataPtr selfPtr, double y)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
}

//.............................................................................

int
StdLib::Printf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);
	return (int) getMainWindow ()->writeOutput_va (format, va.m_va);
}

void
StdLib::testPtr (jnc::DataPtr ptr)
{
	printf ("StdLib::testPtr\n");
}

//.............................................................................

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
	createMdiArea();
	setCentralWidget(mdiArea);

	createActions();
	createMenu();
	createToolBars();
	createPanes();
	createStatusBar();

	readSettings();

	connect(
		this, SIGNAL (outputSignal ()),
		this, SLOT (outputSlot ()),
		Qt::QueuedConnection
		);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	writeSettings();

	mdiArea->closeAllSubWindows();
	if (mdiArea->currentSubWindow()) {
		e->ignore();
	} else {
		e->accept();
	}
}

void MainWindow::newFile()
{
	MdiChild *child = createMdiChild();
	child->newFile();
	child->showMaximized();
}

void MainWindow::openFile(QString filePath)
{
	if (filePath.isEmpty())
	{
		filePath = QFileDialog::getOpenFileName(
			this, 
			"Open File", 
			m_lastDir,
			"Jancy Files (*.jnc);;All Files (*.*)"
			);
	}

	if (filePath.isEmpty())
		return;

	m_lastDir = QFileInfo (filePath).dir ().absolutePath ();

	QMdiSubWindow *subWindow = findMdiSubWindow(filePath);
	if(subWindow) {
		mdiArea->setActiveSubWindow(subWindow);
	} else {
		MdiChild *child = createMdiChild();
		if (child->loadFile(filePath)) {
			writeStatus("File loaded", 2000);
			child->showMaximized();
		} else {
			child->close();
		}
	}
}

void MainWindow::saveFile()
{
	 if (MdiChild* mdiChild = activeMdiChild())
		 if (mdiChild->save())
			 writeStatus("File saved", 2000);
}

void MainWindow::saveAs()
{
	if (MdiChild* mdiChild = activeMdiChild())
		 if (mdiChild->saveAs())
			 writeStatus("File saved", 2000);
}

void MainWindow::createActions()
{
	newFileAction = new QAction(QIcon(":/Images/New"), "&New", this);
	newFileAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
	QObject::connect(newFileAction, SIGNAL(triggered()),
		this, SLOT(newFile()));

	openFileAction = new QAction(QIcon(":/Images/Open"), "&Open", this);
	QObject::connect(openFileAction, SIGNAL(triggered()),
		this, SLOT(openFile()));

	saveFileAction = new QAction(QIcon(":/Images/Save"), "&Save", this);
	saveFileAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	QObject::connect(saveFileAction, SIGNAL(triggered()),
		this, SLOT(saveFile()));

	saveAsAction = new QAction("S&ave as...", this);
	QObject::connect(saveAsAction, SIGNAL(triggered()),
		this, SLOT(saveAs()));

	quitAction = new QAction("&Exit", this);
	QObject::connect(quitAction, SIGNAL(triggered()),
		qApp, SLOT(quit()));

	clearOutputAction = new QAction("&Clear Output", this);
	QObject::connect(clearOutputAction, SIGNAL(triggered()),
		this, SLOT(clearOutput()));

	compileAction = new QAction(QIcon(":/Images/Compile"), "C&ompile", this);
	compileAction->setShortcut(QKeySequence(Qt::Key_F7));
	QObject::connect(compileAction, SIGNAL(triggered()),
		this, SLOT(compile()));

	runAction = new QAction(QIcon(":/Images/Run"), "&Run", this);
	runAction->setShortcut(QKeySequence(Qt::Key_F5));
	QObject::connect(runAction, SIGNAL(triggered()),
		this, SLOT(run()));
}

void MainWindow::createMenu()
{
	fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(newFileAction);
	fileMenu->addAction(openFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(saveFileAction);
	fileMenu->addAction(saveAsAction);
	fileMenu->addSeparator();
	fileMenu->addAction(quitAction);

	editMenu = menuBar()->addMenu("&Edit");
	editMenu->addAction(clearOutputAction);

	debugMenu = menuBar()->addMenu("&Debug");
	debugMenu->addAction(compileAction);
	debugMenu->addAction(runAction);

	viewMenu = menuBar()->addMenu("&View");
}

void MainWindow::createToolBars()
{
	mainToolBar = addToolBar("Main Toolbar");
	mainToolBar->addAction(newFileAction);
	mainToolBar->addAction(openFileAction);
	mainToolBar->addAction(saveFileAction);
	mainToolBar->addSeparator();
	mainToolBar->addAction(compileAction);
	mainToolBar->addAction(runAction);

	viewMenu->addAction(mainToolBar->toggleViewAction());
}

void MainWindow::createStatusBar()
{
	writeStatus("Ready");
}

void MainWindow::createMdiArea()
{
	mdiArea = new QMdiArea(this);
	mdiArea->setViewMode(QMdiArea::TabbedView);
	mdiArea->setTabShape(QTabWidget::Triangular);
	mdiArea->setTabsClosable(true);
	mdiArea->setTabsMovable(true);

	QTabBar *tabBar = mdiArea->findChild<QTabBar *>();
	if (tabBar)
		tabBar->setExpanding(false);
}

void MainWindow::createPanes()
{
	output = new Output(this);
	modulePane = new ModulePane(this);
	llvmIr = new LlvmIr(this);
	disassembly = new Disassembly(this);

	addPane(output, "Output", Qt::BottomDockWidgetArea);
	addPane(modulePane, "Module", Qt::RightDockWidgetArea);

	QDockWidget *llvmIrDock = addPane(llvmIr, "LLVM IR",
		Qt::RightDockWidgetArea);

	QDockWidget *disassemblyDock = addPane(disassembly, "Disassembly",
		Qt::RightDockWidgetArea);

	tabifyDockWidget(llvmIrDock, disassemblyDock);
	llvmIrDock->raise();
}

QDockWidget *MainWindow::addPane(QWidget *widget, const QString &title,
	Qt::DockWidgetArea dockArea)
{
	QDockWidget *dockWidget = new QDockWidget(title, this);
	dockWidget->setWidget(widget);

	addDockWidget(dockArea, dockWidget);

	viewMenu->addAction(dockWidget->toggleViewAction());

	return dockWidget;
}

void MainWindow::writeStatus(const QString &text, int timeout)
{
	statusBar()->showMessage(text, timeout);
}

size_t MainWindow::writeOutputDirect (const char* text, size_t length)
{
	if (length == -1)
		length = strlen (text);

	QString string = QString::fromUtf8 (text, length);

	if (QApplication::instance()->thread () == QThread::currentThread () && outputQueue.empty ())
	{
		output->appendString (string);
		output->repaint ();
	}
	else
	{
		outputMutex.lock ();
		outputQueue.append (string);
		outputMutex.unlock ();

		emit outputSignal ();
	}

	return length;
}

size_t MainWindow::writeOutput_va(const char* format, va_list va)
{
	rtl::String text;
	text.format_va (format, va);
	return writeOutputDirect (text, text.getLength ());
}

size_t MainWindow::writeOutput (const char* format, ...)
{
	va_list va;
	va_start (va, format);
	return writeOutput_va (format, va);
}

void MainWindow::outputSlot ()
{
	outputMutex.lock ();

	while (!outputQueue.empty ())
	{
		QString string = outputQueue.takeFirst ();
		outputMutex.unlock ();

		output->appendString (string);
		output->repaint ();

		outputMutex.lock ();
	}

	outputMutex.unlock ();
}

MdiChild *MainWindow::findMdiChild(const QString &filePath)
{
	MdiChild *child = 0;

	QMdiSubWindow *subWindow = findMdiSubWindow(filePath);
	if (subWindow)
		child = qobject_cast<MdiChild *>(subWindow->widget());

	return child;
}

void MainWindow::readSettings()
{
	QSettings s;

	m_lastDir = s.value ("lastDir").toString ();
	QStringList files = s.value("filesOpened").toStringList();

	foreach (QString file, files)
		openFile(file);
}

void MainWindow::writeSettings()
{
	QSettings s;

	QStringList files;
	foreach (QMdiSubWindow *subWindow, mdiArea->subWindowList())
		if(MdiChild *child = qobject_cast<MdiChild *>(subWindow->widget()))
			files.append(child->file());

	s.setValue("filesOpened", files);
	s.setValue ("lastDir", m_lastDir);
}

jnc::Function *MainWindow::findGlobalFunction(const QString &name)
{
	QByteArray nameBytes = name.toLocal8Bit();
	jnc::ModuleItem *item =
		module.m_namespaceMgr.getGlobalNamespace()->findItem(nameBytes.data());

	if(!item)
		return NULL;

	if(item->getItemKind() != jnc::ModuleItemKind_Function)
		return NULL;

	return (jnc::Function *)item;
}

void MainWindow::clearOutput()
{
	output->clear();
}

//.............................................................................

bool MainWindow::compile ()
{
	qApp->setCursorFlashTime (0);

	bool result;

	MdiChild *child = activeMdiChild();
	if (!child)
		return false;

	if(!child->save())
		return false;

	QByteArray filePathBytes = child->file().toUtf8 ();
	llvm::LLVMContext* pLlvmContext = new llvm::LLVMContext;
	llvm::Module* pLlvmModule = new llvm::Module (filePathBytes.constData(), *pLlvmContext);

	// DebugInfo only works with MCJIT, MCJIT only works on Linux

#if (_AXL_ENV == AXL_ENV_POSIX)
	uint_t ModuleFlags = jnc::ModuleFlag_DebugInfo | jnc::ModuleFlag_McJit;
#else
	uint_t ModuleFlags = 0;
#endif

	module.create (filePathBytes.data(), ModuleFlags);

	jnc::ScopeThreadModule ScopeModule (&module);

	writeOutput("Parsing...\n");

	modulePane->clear ();
	llvmIr->clear ();
	disassembly->clear ();

	QByteArray sourceBytes = child->toPlainText().toUtf8();

	result = module.parse (
		filePathBytes.constData (),
		sourceBytes.constData (),
		sourceBytes.size ()
		);

	if (!result)
	{
		writeOutput("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	writeOutput("Compiling...\n");
	result = module.compile ();
	if (!result)
	{
		writeOutput("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	// TODO: still try to show LLVM IR if calclayout succeeded (and compilation failed somewhere down the road)

	modulePane->build (&module, child);
	llvmIr->build (&module);

	writeOutput("JITting...\n");

	result =
		module.createLlvmExecutionEngine () &&
		StdLib::mapFunctions (&module) &&
		module.jit () &&
		runtime.create (16 * 1024, 16 * 1024) &&
		runtime.addModule (&module); // 16K gc heap, 16K stack

	if (!result)
	{
		writeOutput("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	disassembly->build (&module);

	writeOutput ("Done.\n");
	child->setCompilationNeeded (false);
	return true;
}

bool MainWindow::runFunction (jnc::Function* pFunction, int* pReturnValue)
{
	typedef int FFunction ();
	FFunction* pf = (FFunction*) pFunction->getMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		int ReturnValue = pf ();
		if (pReturnValue)
			*pReturnValue = ReturnValue;

	}
	catch (err::Error Error)
	{
		writeOutput ("ERROR: %s\n", Error.getDescription ().cc ());
		Result = false;
	}
	catch (...)
	{
		writeOutput ("UNKNOWN EXCEPTION\n");
		Result = false;
	}

	return Result;
}

bool
MainWindow::run ()
{
	bool Result;

	 MdiChild* mdiChild = activeMdiChild();
	 if (!mdiChild)
		 return true;

	if (mdiChild->isCompilationNeeded ())
	{
		Result = compile ();
		if (!Result)
			return false;
	}

	jnc::Function* pMainFunction = findGlobalFunction ("main");
	if (!pMainFunction)
	{
		writeOutput ("'main' is not found or not a function\n");
		return false;
	}

	writeOutput ("Running...\n");

	jnc::ScopeThreadRuntime ScopeRuntime (&runtime);

	runtime.startup ();

	// constructor

	jnc::Function* pConstructor = module.getConstructor ();
	if (pConstructor)
	{
		Result = runFunction (pConstructor);
		if (!Result)
			return false;
	}

	// main

	int ReturnValue;
	Result = runFunction (pMainFunction, &ReturnValue);
	if (!Result)
		return false;

	// destructor

	jnc::Function* pDestructor = module.getDestructor ();
	if (pDestructor)
	{
		Result = runFunction (pDestructor);
		if (!Result)
			return false;
	}

	runtime.shutdown ();

	writeOutput ("Done (retval = %d).\n", ReturnValue);
	return true;
}

MdiChild *MainWindow::createMdiChild()
{
	MdiChild *child = new MdiChild(this);
	child->setAttribute(Qt::WA_DeleteOnClose);
	mdiArea->addSubWindow(child);

	return child;
}

MdiChild *MainWindow::activeMdiChild()
{
	 QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow();

	 if (!activeSubWindow && !mdiArea->subWindowList().empty())
		 activeSubWindow = mdiArea->subWindowList().at(0);

	 if (!activeSubWindow)
		 return 0;

	 return qobject_cast<MdiChild *>(activeSubWindow->widget());
}

QMdiSubWindow *MainWindow::findMdiSubWindow(const QString &filePath)
{
	QString canonicalFilePath = QFileInfo(filePath).canonicalFilePath();

	foreach (QMdiSubWindow *subWindow, mdiArea->subWindowList()) {
		MdiChild *child = qobject_cast<MdiChild *>(subWindow->widget());
		if(child && child->file() == canonicalFilePath)
			return subWindow;
	}

	return 0;
}
