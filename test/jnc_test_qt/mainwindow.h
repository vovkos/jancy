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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class LlvmIr;
class ModulePane;
class Output;
class MdiChild;

//..............................................................................

class MainWindow : public QMainWindow
{
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

private:
	QMdiArea* m_mdiArea;
	QString m_lastDir;

	Output* m_output;
	ModulePane* m_modulePane;
	LlvmIr* m_llvmIr;

	QMutex m_outputMutex;
	QStringList m_outputQueue;

	QMenu* m_fileMenu;
	QMenu* m_editMenu;
	QMenu* m_debugMenu;
	QMenu* m_viewMenu;

	QToolBar* m_mainToolBar;

	QAction* m_quitAction;
	QAction* m_newFileAction;
	QAction* m_openFileAction;
	QAction* m_saveFileAction;
	QAction* m_saveAsAction;
	QAction* m_clearOutputAction;
	QAction* m_compileAction;
	QAction* m_runAction;

	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE MainWindow* getMainWindow()
{
	extern MainWindow* g_mainWindow;
	return g_mainWindow;
}

//..............................................................................

#endif
