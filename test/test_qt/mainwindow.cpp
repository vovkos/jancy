#include "pch.h"
#include "mainwindow.h"
#include "mdichild.h"
#include "output.h"
#include "modulepane.h"
#include "llvmir.h"
#include "moc_mainwindow.cpp"
#include "qrc_jancyedit.cpp"

#include "axl_g_WarningSuppression.h"

//.............................................................................

void
AXL_CDECL
TestClassA::foo (int x)
{
	printf ("TestClassA::foo (%d)\n", x);
	m_x = x;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
AXL_CDECL
TestClassB::markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap)
{
//	if (self->m_hiddenIface)
//		self->m_hiddenIface->m_box->gcMarkObject (gcHeap);
}

bool
AXL_CDECL
TestClassB::bar (
	jnc::rt::DataPtr ptr1,
	jnc::rt::DataPtr ptr2,
	jnc::rt::DataPtr ptr3,
	jnc::rt::DataPtr ptr4,
	int a,
	int b
	)
{
	const char* p1 = (const char*) ptr1.m_p;
	const char* p2 = (const char*) ptr2.m_p;
	const char* p3 = (const char*) ptr3.m_p;
	const char* p4 = (const char*) ptr4.m_p;

	printf ("TestClassB::bar ()\n");

	return true;
}

//.............................................................................

void
AXL_CDECL
TestStruct::construct_0 (jnc::rt::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::construct_1 (jnc::rt::DataPtr selfPtr, int x)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
	self->m_x = x;
}

void
AXL_CDECL
TestStruct::construct_2 (jnc::rt::DataPtr selfPtr, double y)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
	self->m_y = y;
}

void
AXL_CDECL
TestStruct::foo_0 (jnc::rt::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::foo_1 (jnc::rt::DataPtr selfPtr, int x)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::foo_2 (jnc::rt::DataPtr selfPtr, double y)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
}

//.............................................................................

int
TestLib::printf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);
	return (int) getMainWindow ()->writeOutput_va (format, va.m_va);
}

void
TestLib::testPtr (
	jnc::rt::DataPtr ptr,
	jnc::rt::DataPtr ptr2
	)
{
	printf ("TestLib::testPtr\n");

	((axl::io::SockAddr*) ptr.m_p)->parse ((const char*) ptr2.m_p);
}

void
TestLib::testVariant (jnc::rt::Variant variant)
{
	printf ("TestLib::testVariant\n");
}

void
TestLib::qtWait (uint_t msTime)
{
	uint64_t start = sys::getTimestamp ();
	uint64_t interval = msTime * 10000;
	
	QEventLoop eventLoop;

	for (;;)
	{
		uint_t now = sys::getTimestamp ();
		if (now - start > interval)
			break;

		eventLoop.processEvents (QEventLoop::AllEvents, 100);
	}
}

//.............................................................................

MainWindow* g_mainWindow = NULL;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
	ASSERT (!g_mainWindow);
	g_mainWindow = this;

	createMdiArea ();
	setCentralWidget (m_mdiArea);

	createActions ();
	createMenu ();
	createToolBars ();
	createPanes ();
	createStatusBar ();

	readSettings ();

	connect (
		this, SIGNAL (outputSignal ()),
		this, SLOT (outputSlot ()),
		Qt::QueuedConnection
		);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	writeSettings ();
	m_mdiArea->closeAllSubWindows ();

	if (m_mdiArea->currentSubWindow ())
		e->ignore();
	else
		e->accept();
}

void MainWindow::newFile()
{
	MdiChild* child = createMdiChild();
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

	QMdiSubWindow* subWindow = findMdiSubWindow(filePath);
	if (subWindow) 
	{
		m_mdiArea->setActiveSubWindow(subWindow);
	} 
	else 
	{
		MdiChild* child = createMdiChild();
		if (child->loadFile(filePath)) 
		{
			writeStatus("File loaded", 2000);
			child->showMaximized();
		} 
		else 
		{
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
	m_newFileAction = new QAction(QIcon(":/Images/New"), "&New", this);
	m_newFileAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
	QObject::connect(m_newFileAction, SIGNAL(triggered()), this, SLOT(newFile()));

	m_openFileAction = new QAction(QIcon(":/Images/Open"), "&Open", this);
	QObject::connect(m_openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	m_saveFileAction = new QAction(QIcon(":/Images/Save"), "&Save", this);
	m_saveFileAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	QObject::connect(m_saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	m_saveAsAction = new QAction("S&ave as...", this);
	QObject::connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	m_quitAction = new QAction("&Exit", this);
	QObject::connect(m_quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	m_clearOutputAction = new QAction("&Clear Output", this);
	QObject::connect(m_clearOutputAction, SIGNAL(triggered()), this, SLOT(clearOutput()));

	m_compileAction = new QAction(QIcon(":/Images/Compile"), "C&ompile", this);
	m_compileAction->setShortcut(QKeySequence(Qt::Key_F7));
	QObject::connect(m_compileAction, SIGNAL(triggered()), this, SLOT(compile()));

	m_runAction = new QAction(QIcon(":/Images/Run"), "&Run", this);
	m_runAction->setShortcut(QKeySequence(Qt::Key_F5));
	QObject::connect(m_runAction, SIGNAL(triggered()), this, SLOT(run()));
}

void MainWindow::createMenu()
{
	m_fileMenu = menuBar()->addMenu("&File");
	m_fileMenu->addAction(m_newFileAction);
	m_fileMenu->addAction(m_openFileAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_saveFileAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_quitAction);

	m_editMenu = menuBar()->addMenu("&Edit");
	m_editMenu->addAction(m_clearOutputAction);

	m_debugMenu = menuBar()->addMenu("&Debug");
	m_debugMenu->addAction(m_compileAction);
	m_debugMenu->addAction(m_runAction);

	m_viewMenu = menuBar()->addMenu("&View");
}

void MainWindow::createToolBars()
{
	m_mainToolBar = addToolBar("Main Toolbar");
	m_mainToolBar->addAction(m_newFileAction);
	m_mainToolBar->addAction(m_openFileAction);
	m_mainToolBar->addAction(m_saveFileAction);
	m_mainToolBar->addSeparator();
	m_mainToolBar->addAction(m_compileAction);
	m_mainToolBar->addAction(m_runAction);

	m_viewMenu->addAction(m_mainToolBar->toggleViewAction());
}

void MainWindow::createStatusBar()
{
	writeStatus("Ready");
}

void MainWindow::createMdiArea()
{
	m_mdiArea = new QMdiArea(this);
	m_mdiArea->setViewMode(QMdiArea::TabbedView);
	m_mdiArea->setTabShape(QTabWidget::Triangular);
	m_mdiArea->setTabsClosable(true);
	m_mdiArea->setTabsMovable(true);

	QTabBar* tabBar = m_mdiArea->findChild<QTabBar*>();
	if (tabBar)
		tabBar->setExpanding(false);
}

void MainWindow::createPanes()
{
	m_output = new Output(this);
	m_modulePane = new ModulePane(this);
	m_llvmIr = new LlvmIr(this);

	addPane(m_output, "Output", Qt::BottomDockWidgetArea);
	addPane(m_modulePane, "Module", Qt::RightDockWidgetArea);
	addPane(m_llvmIr, "LLVM IR", Qt::RightDockWidgetArea);
}

QDockWidget* MainWindow::addPane(QWidget* widget, const QString& title,
	Qt::DockWidgetArea dockArea)
{
	QDockWidget* dockWidget = new QDockWidget(title, this);
	dockWidget->setWidget(widget);

	addDockWidget(dockArea, dockWidget);

	m_viewMenu->addAction(dockWidget->toggleViewAction());

	return dockWidget;
}

void MainWindow::writeStatus(const QString& text, int timeout)
{
	statusBar()->showMessage(text, timeout);
}

size_t MainWindow::writeOutputDirect (const char* text, size_t length)
{
	if (length == -1)
		length = strlen (text);

	QString string = QString::fromUtf8 (text, length);

	if (QApplication::instance()->thread () == QThread::currentThread () && m_outputQueue.empty ())
	{
		m_output->appendString (string);
		m_output->repaint ();
	}
	else
	{
		m_outputMutex.lock ();
		m_outputQueue.append (string);
		m_outputMutex.unlock ();

		emit outputSignal ();
	}

	return length;
}

size_t MainWindow::writeOutput_va(const char* format, va_list va)
{
	sl::String text;
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
	m_outputMutex.lock ();

	while (!m_outputQueue.empty ())
	{
		QString string = m_outputQueue.takeFirst ();
		m_outputMutex.unlock ();

		m_output->appendString (string);
		m_output->repaint ();

		m_outputMutex.lock ();
	}

	m_outputMutex.unlock ();
}

MdiChild* MainWindow::findMdiChild(const QString& filePath)
{
	MdiChild* child = 0;

	QMdiSubWindow* subWindow = findMdiSubWindow(filePath);
	if (subWindow)
		child = qobject_cast<MdiChild*>(subWindow->widget());

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
	foreach (QMdiSubWindow* subWindow, m_mdiArea->subWindowList())
		if(MdiChild* child = qobject_cast<MdiChild*>(subWindow->widget()))
			files.append(child->file());

	s.setValue("filesOpened", files);
	s.setValue ("lastDir", m_lastDir);
}

jnc::ct::Function* MainWindow::findGlobalFunction(const QString& name)
{
	QByteArray nameBytes = name.toLocal8Bit();
	jnc::ct::ModuleItem* item = m_module.m_namespaceMgr.getGlobalNamespace()->findItem(nameBytes.data());

	if(!item)
		return NULL;

	if(item->getItemKind() != jnc::ct::ModuleItemKind_Function)
		return NULL;

	return (jnc::ct::Function*)item;
}

void MainWindow::clearOutput()
{
	m_output->clear();
}

//.............................................................................

bool MainWindow::compile ()
{
	qApp->setCursorFlashTime (0);

	bool result;

	MdiChild* child = activeMdiChild();
	if (!child)
		return false;

	if(!child->save())
		return false;

	// DebugInfo only works with MCJIT, MCJIT only works on Linux

#if (_AXL_ENV == AXL_ENV_POSIX)
	uint_t compileFlags = jnc::ct::ModuleCompileFlag_StdFlags | jnc::ct::ModuleCompileFlag_DebugInfo;
#else
	uint_t compileFlags = jnc::ct::ModuleCompileFlag_StdFlags;
#endif

//	compileFlags |= jnc::ct::ModuleCompileFlag_SimpleGcSafePoint;

	QByteArray sourceFilePath = child->file().toUtf8 ();
	QByteArray appDir = qApp->applicationDirPath ().toUtf8 ();

	jnc::ext::ExtensionLibHost* libHost = jnc::ext::getStdExtensionLibHost ();

	result = 
		m_module.create (sourceFilePath.data(), compileFlags) &&
		m_module.m_extensionLibMgr.addStaticLib (jnc::ext::getStdLib (libHost)) &&
		m_module.m_extensionLibMgr.addStaticLib (sl::getSimpleSingleton <TestLib> ());

	m_module.m_importMgr.m_importDirList.insertTail (appDir.constData ());

	writeOutput("Parsing...\n");

	m_modulePane->clear ();
	m_llvmIr->clear ();

	QByteArray source = child->toPlainText().toUtf8();

	result = m_module.parse (
		sourceFilePath.constData (),
		source.constData (),
		source.size ()
		) &&
		m_module.parseImports ();

	if (!result)
	{
		writeOutput("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	writeOutput("Compiling...\n");
	result = m_module.compile ();
	if (!result)
	{
		writeOutput("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	// TODO: still try to show LLVM IR if calclayout succeeded (and compilation failed somewhere down the road)

	m_modulePane->build (&m_module, child);
	m_llvmIr->build (&m_module);

	writeOutput("JITting...\n");

	result =
		m_module.createLlvmExecutionEngine () &&
		m_module.jit ();

	if (!result)
	{
		writeOutput("%s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	writeOutput ("Done.\n");
	child->setCompilationNeeded (false);
	return true;
}

bool
MainWindow::run ()
{
	bool result;

	 MdiChild* mdiChild = activeMdiChild ();
	 if (!mdiChild)
		 return true;

	if (mdiChild->isCompilationNeeded ())
	{
		result = compile ();
		if (!result)
			return false;
	}

	jnc::ct::Function* mainFunction = findGlobalFunction ("main");
	if (!mainFunction)
	{
		writeOutput ("'main' is not found or not a function\n");
		return false;
	}

	writeOutput ("Running...\n");

//	m_runtime.m_gcHeap.setSizeTriggers (-1, -1);
	result = m_runtime.startup (&m_module);
	if (!result)
	{
		writeOutput ("Cannot startup Jancy runtime: %s\n", err::getLastErrorDescription ().cc ());
		return false;
	}

	int returnValue;
	result = jnc::rt::callFunction (&m_runtime, mainFunction, &returnValue);
	if (result)
		writeOutput ("'main' returned %d.\n", returnValue);
	else
		writeOutput ("Runtime error: %s\n", err::getLastErrorDescription ().cc ());
	
	writeOutput ("Shutting down...\n");
	m_runtime.shutdown ();
	writeOutput ("Done.\n");
	return false;
}

MdiChild* MainWindow::createMdiChild ()
{
	MdiChild* child = new MdiChild(this);
	child->setAttribute(Qt::WA_DeleteOnClose);
	m_mdiArea->addSubWindow(child);

	return child;
}

MdiChild* MainWindow::activeMdiChild ()
{
	 QMdiSubWindow* activeSubWindow = m_mdiArea->activeSubWindow();

	 if (!activeSubWindow && !m_mdiArea->subWindowList().empty())
		 activeSubWindow = m_mdiArea->subWindowList().at(0);

	 if (!activeSubWindow)
		 return 0;

	 return qobject_cast<MdiChild*>(activeSubWindow->widget());
}

QMdiSubWindow* MainWindow::findMdiSubWindow (const QString& filePath)
{
	QString canonicalFilePath = QFileInfo(filePath).canonicalFilePath();

	foreach (QMdiSubWindow* subWindow, m_mdiArea->subWindowList()) {
		MdiChild* child = qobject_cast<MdiChild*>(subWindow->widget());
		if(child && child->file() == canonicalFilePath)
			return subWindow;
	}

	return 0;
}

//.............................................................................
