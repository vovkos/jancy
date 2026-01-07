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
#include "mdichild.h"
#include "output.h"
#include "modulepane.h"
#include "llvmir.h"
#include "testlib.h"
#include "moc_mainwindow.cpp"
#include "qrc_res.cpp"

// #define _NO_GC 1

#define DEFAULT_STDLIB           true
#define DEFAULT_ASSERT           true
#define DEFAULT_DEBUG_INFO       false
#define DEFAULT_OPTIMIZE         false
#define DEFAULT_DISABLE_CODE_GEN false
#define DEFAULT_JIT              true
#define DEFAULT_JIT_KIND         jnc::JitKind_Auto

//..............................................................................

size_t
printToOutput(
	const void* p,
	size_t length
) {
	fwrite(p, length, 1, stdout);
	return (int)getMainWindow()->writeOutputDirect(QString::fromUtf8((const char*)p, length));
}

//..............................................................................

MainWindow* g_mainWindow = NULL;

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags) {
	ASSERT(!g_mainWindow);
	g_mainWindow = this;

#if (_JNC_OS_WIN)
	m_libDir = qApp->applicationDirPath();
#else
#	if (_JNC_DEBUG)
	m_libDir = qApp->applicationDirPath() + "/../../lib/Debug";
#	else
	m_libDir = qApp->applicationDirPath() + "/../../lib/Release";
#	endif
#endif

	m_capabilities = "*";

	m_usbFilter =
		"16d0 0e26 "  // serial tap (MSC)
		"16d0 0e27 "  // i2c-spi tap (MSC)
		"326f 0003";  // ethernet tap

	m_module->setCompileErrorHandler(compileErrorHandler, this);

	createMdiArea();
	setCentralWidget(m_mdiArea);

	createActions();
	createMenu();
	createToolBars();
	createPanes();
	createStatusBar();

	readSettings();

	connect(
		this, SIGNAL(outputSignal()),
		this, SLOT(outputSlot()),
		Qt::QueuedConnection
	);

	jnc::StdLib_setStdIo(NULL, printToOutput, printToOutput);
}

bool_t MainWindow::compileErrorHandler(
	void* context,
	jnc::ModuleCompileErrorKind errorKind
) {
	MainWindow* self = (MainWindow*)context;
	self->writeOutput("%s\n", jnc::getLastErrorDescription_v());
	return true;
}

void MainWindow::closeEvent(QCloseEvent* e) {
	writeSettings();
	m_mdiArea->closeAllSubWindows();

	if (!m_mdiArea->subWindowList().isEmpty())
		e->ignore();
	else
		e->accept();
}

void MainWindow::newFile() {
	MdiChild* child = createMdiChild();
	child->newFile();
	child->showMaximized();
}

void MainWindow::openFile(QString filePath) {
	if (filePath.isEmpty()) {
		filePath = QFileDialog::getOpenFileName(
			this,
			"Open File",
			m_lastDir,
			"Jancy Files (*.jnc);;All Files (*.*)"
		);
	}

	if (filePath.isEmpty())
		return;

	m_lastDir = QFileInfo(filePath).dir().absolutePath();

	QMdiSubWindow* subWindow = findMdiSubWindow(filePath);
	if (subWindow) {
		m_mdiArea->setActiveSubWindow(subWindow);
	} else {
		MdiChild* child = createMdiChild();
		if (child->loadFile(filePath)) {
			writeStatus("File loaded", 2000);
			child->showMaximized();
		} else {
			child->close();
		}
	}
}

void MainWindow::saveFile() {
	if (MdiChild* mdiChild = activeMdiChild())
		if (mdiChild->save())
			writeStatus("File saved", 2000);
}

void MainWindow::saveAs() {
	if (MdiChild* mdiChild = activeMdiChild())
		if (mdiChild->saveAs())
			writeStatus("File saved", 2000);
}

void MainWindow::createActions() {
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

	m_syntaxHighlightingAction = new QAction("Highlight Jancy &Syntax", this);
	m_syntaxHighlightingAction->setCheckable(true);
	m_syntaxHighlightingAction->setChecked(true);
	QObject::connect(m_syntaxHighlightingAction, SIGNAL(triggered(bool)), this, SLOT(onEnableSyntaxHighlighting(bool)));

	m_currentLineHighlightingAction = new QAction("Highlight &Current line", this);
	m_currentLineHighlightingAction->setCheckable(true);
	m_currentLineHighlightingAction->setChecked(true);
	QObject::connect(m_currentLineHighlightingAction, SIGNAL(triggered(bool)), this, SLOT(onEnableCurrentLineHighlighting(bool)));

	m_lineNumberMarginAction = new QAction("Line &Number Margin", this);
	m_lineNumberMarginAction->setCheckable(true);
	m_lineNumberMarginAction->setChecked(true);
	QObject::connect(m_lineNumberMarginAction, SIGNAL(triggered(bool)), this, SLOT(onEnableLineNumberMargin(bool)));

	m_stdlibAction = new QAction("Standard &Libraries", this);
	m_stdlibAction->setCheckable(true);
	m_stdlibAction->setChecked(DEFAULT_STDLIB);

	m_assertAction = new QAction("&Asserts", this);
	m_assertAction->setCheckable(true);
	m_assertAction->setChecked(DEFAULT_ASSERT);

	m_signedExtensionsAction = new QAction("&Signed Extensions", this);
	m_signedExtensionsAction->setCheckable(true);

	m_simpleGcSafePointAction = new QAction("&Simple GC Safe-point", this);
	m_simpleGcSafePointAction->setCheckable(true);
	m_simpleGcSafePointAction->setChecked(false);

	m_disableCodeGenAction = new QAction("E&xclude Code Generation", this);
	m_disableCodeGenAction->setCheckable(true);
	m_disableCodeGenAction->setChecked(DEFAULT_DISABLE_CODE_GEN);

	m_debugInfoAction = new QAction("&Debug Info", this);
	m_debugInfoAction->setCheckable(true);
	m_debugInfoAction->setChecked(DEFAULT_DEBUG_INFO);

	m_optimizeAction = new QAction("&Optimize", this);
	m_optimizeAction->setCheckable(true);
	m_optimizeAction->setChecked(DEFAULT_OPTIMIZE);

	m_jitAction = new QAction("&JIT", this);
	m_jitAction->setCheckable(true);
	m_jitAction->setChecked(DEFAULT_JIT);

	m_mcJitAction = new QAction("MCJIT", this);
	m_mcJitAction->setCheckable(true);
	m_mcJitAction->setChecked(DEFAULT_JIT_KIND == jnc::JitKind_McJit);
#if (_JNC_LLVM_JIT_ORC)
	m_orcJitAction = new QAction("ORC JIT", this);
	m_orcJitAction->setCheckable(true);
	m_orcJitAction->setChecked(DEFAULT_JIT_KIND == jnc::JitKind_Orc);
#endif
#if (_JNC_LLVM_JIT_LEGACY)
	m_legacyJitAction = new QAction("Legacy JIT", this);
	m_legacyJitAction->setCheckable(true);
	m_legacyJitAction->setChecked(DEFAULT_JIT_KIND == jnc::JitKind_Legacy);
#endif

	m_setCapabilitiesAction = new QAction("Set Capabilities", this);
	QObject::connect(m_setCapabilitiesAction, SIGNAL(triggered()), this, SLOT(onSetCapabilities()));

	m_setUsbFilterAction = new QAction("Set USB Filter", this);
	QObject::connect(m_setUsbFilterAction, SIGNAL(triggered()), this, SLOT(onSetUsbFilter()));

	m_compileAction = new QAction(QIcon(":/Images/Compile"), "C&ompile", this);
	m_compileAction->setShortcut(QKeySequence(Qt::Key_F7));
	QObject::connect(m_compileAction, SIGNAL(triggered()), this, SLOT(compile()));

	m_runAction = new QAction(QIcon(":/Images/Run"), "&Run", this);
	m_runAction->setShortcut(QKeySequence(Qt::Key_F5));
	QObject::connect(m_runAction, SIGNAL(triggered()), this, SLOT(run()));
}

void MainWindow::createMenu() {
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

	m_compileMenu = menuBar()->addMenu("&Compile");
	m_compileMenu->addAction(m_stdlibAction);
	m_compileMenu->addAction(m_assertAction);
	m_compileMenu->addAction(m_signedExtensionsAction);
	m_compileMenu->addAction(m_simpleGcSafePointAction);
	m_compileMenu->addAction(m_disableCodeGenAction);
	m_compileMenu->addAction(m_debugInfoAction);
	m_compileMenu->addAction(m_optimizeAction);
	m_compileMenu->addAction(m_jitAction);

	QActionGroup* group = new QActionGroup(this);
	group->addAction(m_mcJitAction);
#if (_JNC_LLVM_JIT_ORC)
	group->addAction(m_orcJitAction);
#endif
#if (_JNC_LLVM_JIT_LEGACY)
	group->addAction(m_legacyJitAction);
#endif

	m_compileMenu->addSeparator();
	m_compileMenu->addActions(group->actions());
	m_compileMenu->addSeparator();
	m_compileMenu->addAction(m_setCapabilitiesAction);
	m_compileMenu->addAction(m_setUsbFilterAction);
	m_compileMenu->addSeparator();
	m_compileMenu->addAction(m_compileAction);
	m_compileMenu->addAction(m_runAction);

	m_viewMenu = menuBar()->addMenu("&View");
	m_viewMenu->addAction(m_syntaxHighlightingAction);
	m_viewMenu->addAction(m_currentLineHighlightingAction);
	m_viewMenu->addAction(m_lineNumberMarginAction);
	m_viewMenu->addSeparator();
}

void MainWindow::createToolBars() {
	m_mainToolBar = addToolBar("Main Toolbar");
	m_mainToolBar->addAction(m_newFileAction);
	m_mainToolBar->addAction(m_openFileAction);
	m_mainToolBar->addAction(m_saveFileAction);
	m_mainToolBar->addSeparator();
	m_mainToolBar->addAction(m_compileAction);
	m_mainToolBar->addAction(m_runAction);

	m_viewMenu->addAction(m_mainToolBar->toggleViewAction());
}

void MainWindow::createStatusBar() {
	writeStatus("Ready");
}

void MainWindow::createMdiArea() {
	m_mdiArea = new QMdiArea(this);
	m_mdiArea->setViewMode(QMdiArea::TabbedView);
	m_mdiArea->setTabShape(QTabWidget::Triangular);
	m_mdiArea->setTabsClosable(true);
	m_mdiArea->setTabsMovable(true);

	QTabBar* tabBar = m_mdiArea->findChild<QTabBar*>();
	if (tabBar)
		tabBar->setExpanding(false);
}

void MainWindow::createPanes() {
	m_output = new Output(this);
	m_modulePane = new ModulePane(this);
	m_llvmIr = new LlvmIr(this);

	addPane(m_output, "Output", Qt::BottomDockWidgetArea);
	addPane(m_modulePane, "Module", Qt::RightDockWidgetArea);
	addPane(m_llvmIr, "LLVM IR", Qt::RightDockWidgetArea);
}

QDockWidget* MainWindow::addPane(QWidget* widget, const QString& title,
	Qt::DockWidgetArea dockArea) {
	QDockWidget* dockWidget = new QDockWidget(title, this);
	dockWidget->setWidget(widget);

	addDockWidget(dockArea, dockWidget);

	m_viewMenu->addAction(dockWidget->toggleViewAction());

	return dockWidget;
}

void MainWindow::writeStatus(const QString& text, int timeout) {
	statusBar()->showMessage(text, timeout);
}

size_t MainWindow::writeOutputDirect(const QString& string) {
	if (QApplication::instance()->thread() == QThread::currentThread() && m_outputQueue.empty()) {
		m_output->appendString(string);
		m_output->repaint();
	} else {
		m_outputMutex.lock();
		m_outputQueue.append(string);
		m_outputMutex.unlock();

		emit outputSignal();
	}

	return string.length();
}

size_t MainWindow::writeOutput_va(const char* format, va_list va) {
	QString text;
	text.vsprintf(format, va);
	return writeOutputDirect(text);
}

size_t MainWindow::writeOutput(const char* format, ...) {
	va_list va;
	va_start(va, format);
	return writeOutput_va(format, va);
}

void MainWindow::outputSlot() {
	m_outputMutex.lock();

	while (!m_outputQueue.empty()) {
		QString string = m_outputQueue.takeFirst();
		m_outputMutex.unlock();

		m_output->appendString(string);
		m_output->repaint();

		m_outputMutex.lock();
	}

	m_outputMutex.unlock();
}

void MainWindow::onEnableSyntaxHighlighting(bool isEnabled) {
	MdiChild* child = activeMdiChild();
	if (child)
		child->enableSyntaxHighlighting(isEnabled);
}

void MainWindow::onEnableCurrentLineHighlighting(bool isEnabled) {
	MdiChild* child = activeMdiChild();
	if (child)
		child->enableCurrentLineHighlighting(isEnabled);
}

void MainWindow::onEnableLineNumberMargin(bool isEnabled) {
	MdiChild* child = activeMdiChild();
	if (child)
		child->enableLineNumberMargin(isEnabled);
}

MdiChild* MainWindow::findMdiChild(const QString& filePath) {
	MdiChild* child = 0;

	QMdiSubWindow* subWindow = findMdiSubWindow(filePath);
	if (subWindow)
		child = qobject_cast<MdiChild*>(subWindow->widget());

	return child;
}

void MainWindow::readSettings() {
	QSettings s;

	m_lastDir = s.value("lastDir").toString();
	QStringList files = s.value("filesOpened").toStringList();

	foreach(QString file, files)
		openFile(file);
}

void MainWindow::writeSettings() {
	QSettings s;

	QStringList files;
	foreach(QMdiSubWindow* subWindow, m_mdiArea->subWindowList())
		if(MdiChild* child = qobject_cast<MdiChild*>(subWindow->widget()))
			files.append(child->filePath());

	s.setValue("filesOpened", files);
	s.setValue("lastDir", m_lastDir);
}

jnc::Function* MainWindow::findGlobalFunction(const QString& name) {
	QByteArray nameBytes = name.toLocal8Bit();
	jnc::FindModuleItemResult findResult = m_module->getGlobalNamespace()->getNamespace()->findItem(nameBytes.data());
	return findResult.m_item && findResult.m_item->getItemKind() == jnc::ModuleItemKind_Function ?
		(jnc::Function*)findResult.m_item :
		NULL;
}

void MainWindow::clearOutput() {
	m_output->clear();
}

//..............................................................................

void MainWindow::onSetCapabilities() {
	QInputDialog inputDialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
	inputDialog.setWindowTitle("Set Capabilities");
	inputDialog.setInputMode(QInputDialog::TextInput);
	inputDialog.setLabelText("Capabilities:");
	inputDialog.setTextValue(m_capabilities);
	inputDialog.resize(QSize(640, inputDialog.height()));

	int result = inputDialog.exec();
	if (result != QDialog::Accepted)
		return;

	m_capabilities = inputDialog.textValue();
	jnc::initializeCapabilities(m_capabilities.toLatin1().data());
}

void MainWindow::onSetUsbFilter() {
	QInputDialog inputDialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
	inputDialog.setWindowTitle("Set Allowed USB Devices");
	inputDialog.setInputMode(QInputDialog::TextInput);
	inputDialog.setLabelText("USB VIDs/PIDs:");
	inputDialog.setTextValue(m_usbFilter);
	inputDialog.resize(QSize(640, inputDialog.height()));

	int result = inputDialog.exec();
	if (result != QDialog::Accepted)
		return;

	m_usbFilter = inputDialog.textValue();

	QVector<uint16_t> vidPidTable;
	QByteArray string = m_usbFilter.toLatin1();
	const char* p = string.data();
	const char* end = string.end();
	while (p < end) {
		char* next;
		uint16_t id = strtoul(p, &next, 16);
		if (p == next)
			break;

		vidPidTable.append(id);
		p = next;
	}

	jnc::writeCapabilityParam(
		"org.jancy.io.usb.devices",
		vidPidTable.data(),
		vidPidTable.size() * sizeof(uint16_t)
	);
}

bool MainWindow::compile() {
	bool result;

	MdiChild* child = activeMdiChild();
	if (!child)
		return false;

	if(!child->save())
		return false;

	writeOutput("Compiling...\n");

	// DebugInfo does not work on Windows

	jnc::ModuleConfig moduleConfig = jnc::g_defaultModuleConfig;

	if (m_simpleGcSafePointAction->isChecked())
		moduleConfig.m_compileFlags |= jnc::ModuleCompileFlag_SimpleGcSafePoint;

	if (m_disableCodeGenAction->isChecked())
		moduleConfig.m_compileFlags |= jnc::ModuleCompileFlag_DisableCodeGen;

#if (!_JNC_OS_WIN)
	if (m_debugInfoAction->isChecked())
		moduleConfig.m_compileFlags |= jnc::ModuleCompileFlag_DebugInfo;
#endif

	if (m_assertAction->isChecked())
		moduleConfig.m_compileFlags |= jnc::ModuleCompileFlag_Assert;

	if (m_mcJitAction->isChecked())
		moduleConfig.m_jitKind = jnc::JitKind_McJit;
#if (_JNC_LLVM_JIT_ORC)
	if (m_orcJitAction->isChecked())
		moduleConfig.m_jitKind = jnc::JitKind_Orc;
#endif
#if (_JNC_LLVM_JIT_LEGACY)
	if (m_legacyJitAction->isChecked())
		moduleConfig.m_jitKind = jnc::JitKind_Legacy;
#endif

	QByteArray sourceFilePath = child->filePath().toUtf8();

	m_module->initialize(sourceFilePath.data(), &moduleConfig);

	if (m_stdlibAction->isChecked()) {
		m_module->addStaticLib(jnc::StdLib_getLib());
		m_module->addStaticLib(jnc::SysLib_getLib());
		m_module->addImportDir(m_libDir.toUtf8().constData());
	}

	m_module->addStaticLib(TestLib_getLib());

	if (m_signedExtensionsAction->isChecked()) {
		jnc::CodeAuthenticatorConfig config;
#if (_JNC_OS_WIN)
		config.m_expectedSubjectName = "Tibbo Technology Inc.";
		config.m_expectedIssuerName = "DigiCert EV Code Signing CA";
#elif (_JNC_OS_LINUX)
		config.m_signatureSectionName = ".njsig";
		config.m_publicKeyPem =
			"-----BEGIN PUBLIC KEY-----\n"
			"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0asmW7hwuqlO6XWkpCCz\n"
			"bs/S8JeTn8cpHbwskgRfroK7PR1OOhyZA0CjsB46tau1w0WLeKRB9MPeCn1aufaU\n"
			"nEQ09Dmqy/8Nhjrzm58AclKWO8wT3qhN2tDOh2r5Kw53w5vXxLysJ1sj6wRMnREq\n"
			"2AWqLsHGpI/jk93+znPPvwSte3M8ZBPIukQn/QQ1S289sAlEWol4AJjGmlHDsNDI\n"
			"6dQBZM6uUHejYv5um+YICVhCpcJ4h93pvrRCPpg9JgiCDVxN5dbQ0vDMYKNZAZmL\n"
			"qc2WkGZ3ubiPpDBHwzzleK3S3pLJZFywdpLMRqNuMJuGBxOm4UxWQybkU8vAtJ0O\n"
			"yQIDAQAB\n"
			"-----END PUBLIC KEY-----\n";
#elif (_JNC_OS_DARWIN)
		config.m_expectedTeamId = "MHV447DZEV";
#endif
		m_module->setDynamicExtensionAuthenticatorConfig(&config);
	}

	m_module->require(jnc::ModuleItemKind_Function, "main");

	m_modulePane->clear();
	m_llvmIr->clear();

	QByteArray source = child->toPlainText().toUtf8();

	result =
		m_module->parse(sourceFilePath.constData(), source.constData(), source.size()) &&
		m_module->parseImports() &&
		m_module->compile();

	if (!result) {
		writeOutput("%s\n", jnc::getLastErrorDescription_v());
		return false;
	}

	if (m_optimizeAction->isChecked()) {
		writeOutput("Optimizing...\n");
		result = m_module->optimize();
		if (!result) {
			writeOutput("%s\n", jnc::getLastErrorDescription_v());
			return false;
		}
	}

	// TODO: still try to show LLVM IR if calclayout succeeded (and compilation failed somewhere down the road)

	m_modulePane->build(m_module, child);
	m_llvmIr->build(m_module);

	if (m_jitAction->isChecked()) {
		writeOutput("JITting...\n");

		result = m_module->jit();
		if (!result) {
			writeOutput("%s\n", jnc::getLastErrorDescription_v());
			return false;
		}
	}

	writeOutput("Done.\n");
	child->setCompilationNeeded(false);
	return true;
}

bool
MainWindow::run() {
	bool result;

	MdiChild* mdiChild = activeMdiChild();
	if (!mdiChild)
		return true;

	if (mdiChild->isCompilationNeeded()) {
		result = compile();
		if (!result)
			return false;
	}

	jnc::Function* mainFunction = findGlobalFunction("main");
	if (!mainFunction) {
		writeOutput("'main' is not found or not a function\n");
		return false;
	}

	writeOutput("Running...\n");

#if (_NO_GC)
	jnc::GcSizeTriggers triggers;
	triggers.m_allocSizeTrigger = -1;
	triggers.m_periodSizeTrigger = -1;
	m_runtime->getGcHeap()->setSizeTriggers(&triggers);
#endif

	result = m_runtime->startup(m_module);
	if (!result) {
		writeOutput("Cannot startup Jancy runtime: %s\n", jnc::getLastErrorDescription_v());
		return false;
	}

	int returnValue;
	result = jnc::callFunction(m_runtime, mainFunction, &returnValue);
	if (result)
		writeOutput("'main' returned %d.\n", returnValue);
	else
		writeOutput("Runtime error: %s\n", jnc::getLastErrorDescription_v());

	if (result && returnValue == -1000) { // for testing some async stuff with threads
		writeOutput("Staying resident...\n");
		return true;
	}

	writeOutput("Shutting down...\n");
	m_runtime->shutdown();
	writeOutput("Done.\n");
	return true;
}

MdiChild* MainWindow::createMdiChild() {
	MdiChild* child = new MdiChild(this);
	child->setAttribute(Qt::WA_DeleteOnClose);
	child->setImportDirList(QStringList(m_libDir));
	m_mdiArea->addSubWindow(child);
	return child;
}

MdiChild* MainWindow::activeMdiChild() {
	QMdiSubWindow* activeSubWindow = m_mdiArea->activeSubWindow();

	if (!activeSubWindow && !m_mdiArea->subWindowList().empty())
		activeSubWindow = m_mdiArea->subWindowList().at(0);

	if (!activeSubWindow)
		return 0;

	return qobject_cast<MdiChild*>(activeSubWindow->widget());
}

QMdiSubWindow* MainWindow::findMdiSubWindow(const QString& filePath) {
	QString canonicalFilePath = QFileInfo(filePath).canonicalFilePath();

	foreach(QMdiSubWindow* subWindow, m_mdiArea->subWindowList()) {
		MdiChild* child = qobject_cast<MdiChild*>(subWindow->widget());
		if(child && child->filePath() == canonicalFilePath)
			return subWindow;
	}

	return 0;
}

//..............................................................................
