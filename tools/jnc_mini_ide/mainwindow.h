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

#pragma once

class LlvmIr;
class ModulePane;
class Output;
class MdiChild;

//..............................................................................

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	QSize sizeHint() const { return QSize(800, 600); }

	void writeStatus(const QString &text, int timeout = 0);

	size_t writeOutputDirect(const QString& text);
	size_t writeOutput_va(const char* format, va_list va);
	size_t writeOutput(const char* format, ...);

	MdiChild* findMdiChild(const QString &filePath);

protected:
	void closeEvent(QCloseEvent* e);

private slots:
	void newFile();
	void openFile(QString filePath = "");
	void saveFile();
	void saveAs();
	void clearOutput();
	bool compile();
	bool run();
	void outputSlot();
	void onEnableSyntaxHighlighting(bool isEnabled);
	void onEnableCurrentLineHighlighting(bool isEnabled);
	void onEnableLineNumberMargin(bool isEnabled);
	void onSetCapabilities();
	void onSetUsbFilter();

signals:
	void outputSignal();

private:
	void createActions();
	void createMenu();
	void createToolBars();
	void createStatusBar();
	void createMdiArea();

	void createPanes();
	QDockWidget* addPane(QWidget* widget, const QString &title, Qt::DockWidgetArea dockArea);

	MdiChild* createMdiChild();
	MdiChild* activeMdiChild();
	QMdiSubWindow* findMdiSubWindow(const QString &filePath);
	void readSettings();
	void writeSettings();

	jnc::Function* findGlobalFunction(const QString &name);

	static bool_t compileErrorHandler(
		void* context,
		jnc::ModuleCompileErrorKind errorKind
	);

private:
	QMdiArea* m_mdiArea;
	QString m_lastDir;
	QString m_libDir;
	QString m_capabilities;
	QString m_usbFilter;

	Output* m_output;
	ModulePane* m_modulePane;
	LlvmIr* m_llvmIr;

	QMutex m_outputMutex;
	QStringList m_outputQueue;

	QMenu* m_fileMenu;
	QMenu* m_editMenu;
	QMenu* m_compileMenu;
	QMenu* m_viewMenu;

	QToolBar* m_mainToolBar;

	QAction* m_quitAction;
	QAction* m_newFileAction;
	QAction* m_openFileAction;
	QAction* m_saveFileAction;
	QAction* m_saveAsAction;
	QAction* m_clearOutputAction;
	QAction* m_syntaxHighlightingAction;
	QAction* m_currentLineHighlightingAction;
	QAction* m_lineNumberMarginAction;
	QAction* m_compileAction;
	QAction* m_runAction;
	QAction* m_stdlibAction;
	QAction* m_assertAction;
	QAction* m_simpleGcSafePointAction;
	QAction* m_debugInfoAction;
	QAction* m_disableCodeGenAction;
	QAction* m_mcJitAction;
#if (_JNC_LLVM_JIT_ORC)
	QAction* m_orcJitAction;
#endif
#if (_JNC_LLVM_JIT_LEGACY)
	QAction* m_legacyJitAction;
#endif
	QAction* m_optimizeAction;
	QAction* m_jitAction;
	QAction* m_signedExtensionsAction;
	QAction* m_setCapabilitiesAction;
	QAction* m_setUsbFilterAction;

	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE MainWindow* getMainWindow() {
	extern MainWindow* g_mainWindow;
	return g_mainWindow;
}

//..............................................................................
