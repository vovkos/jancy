#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class Disassembly;
class LlvmIr;
class ModulePane;
class Output;
class MdiChild;

//.............................................................................

struct Point
{
	int64_t m_x;
	int64_t m_y;
	int64_t m_z;
	int64_t m_w;
};

//.............................................................................

enum ApiSlot
{
	ApiSlot_TestClassA,
	ApiSlot_TestClassB,
	ApiSlot_TestStruct,
};

//.............................................................................

class TestClassA: public jnc::IfaceHdr
{
public:
	JNC_BEGIN_CLASS ("TestClassA", ApiSlot_TestClassA)
		JNC_FUNCTION ("foo", &TestClassA::foo)
	JNC_END_CLASS ()

public:
	int m_x;

public:
	void
	AXL_CDECL
	destruct ();

	void
	AXL_CDECL
	foo (int x);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestClassB: public TestClassA
{
public:
	JNC_BEGIN_CLASS ("TestClassB", ApiSlot_TestClassB)
		JNC_OPAQUE_CLASS (TestClassB, &enumGcRoots)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_FUNCTION ("bar", &TestClassB::bar)
	JNC_END_CLASS ()

public:
	int m_y;

public:
	static
	void
	enumGcRoots (
		jnc::Runtime* runtime,
		TestClassB* self
		);

	static 
	TestClassB*
	operatorNew ();

	void
	AXL_CDECL
	bar (int y);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestStruct
{
public:
	JNC_BEGIN_TYPE ("TestStruct", ApiSlot_TestStruct)
		JNC_CONSTRUCTOR (&TestStruct::construct_0)
		JNC_OVERLOAD (&TestStruct::construct_1)
		JNC_OVERLOAD (&TestStruct::construct_2)

		JNC_FUNCTION ("foo", &TestStruct::foo_0)
		JNC_OVERLOAD (&TestStruct::foo_1)
		JNC_OVERLOAD (&TestStruct::foo_2)
	JNC_END_TYPE ()

public:
	int m_x;
	double m_y;

public:
	static
	void
	AXL_CDECL
	construct_0 (jnc::DataPtr selfPtr);

	static
	void
	AXL_CDECL
	construct_1 (
		jnc::DataPtr selfPtr, 
		int x
		);

	static
	void
	AXL_CDECL
	construct_2 (
		jnc::DataPtr selfPtr, 
		double y
		);

	static
	void
	AXL_CDECL
	foo_0 (jnc::DataPtr selfPtr);

	static
	void
	AXL_CDECL
	foo_1 (
		jnc::DataPtr selfPtr, 
		int x
		);

	static
	void
	AXL_CDECL
	foo_2 (
		jnc::DataPtr selfPtr, 
		double y
		);
};

//.............................................................................

class StdLib: public jnc::StdLib
{
public:
	JNC_BEGIN_LIB ()
		JNC_STD_FUNCTION (jnc::StdFunction_Printf,  &Printf)
//		JNC_TYPE (TestClassA)
//		JNC_TYPE (TestClassB)
//		JNC_TYPE (TestStruct)
		JNC_LIB (jnc::StdLib)
//		JNC_FUNCTION ("testPtr",  &testPtr)
	JNC_END_LIB ()

	static
	int
	Printf (
		const char* pFormat,
		...
		);

	static
	void
	testPtr (jnc::DataPtr Ptr);
};

//.............................................................................

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	QSize sizeHint() const { return QSize(800, 600); }

	void writeStatus(const QString &text, int timeout = 0);
	
	size_t writeOutputDirect(const char* text, size_t length = -1);
	size_t writeOutput_va(const char* format, va_list va);
	size_t writeOutput(const char* format, ...);

	MdiChild *findMdiChild(const QString &filePath);

protected:
	void closeEvent(QCloseEvent *e);

private slots:
	void newFile();
	void openFile(QString filePath = "");
	void saveFile();
	void saveAs();
	void clearOutput();
	bool compile();
	bool run();
	void outputSlot ();

signals:
	void outputSignal ();

private:
	void createActions();
	void createMenu();
	void createToolBars();
	void createStatusBar();
	void createMdiArea();

	bool runFunction (jnc::Function* pFunction, int* pReturnValue = NULL);

	void createPanes();
	QDockWidget *addPane(QWidget *widget, const QString &title, Qt::DockWidgetArea dockArea);

	MdiChild *createMdiChild();
	MdiChild *activeMdiChild();
	QMdiSubWindow *findMdiSubWindow(const QString &filePath);
	void readSettings();
	void writeSettings();

	jnc::Function *findGlobalFunction(const QString &name);

	QMdiArea *mdiArea;
	QString m_lastDir;

	Output *output;
	ModulePane *modulePane;
	LlvmIr *llvmIr;
	Disassembly *disassembly;

	QMutex outputMutex;
	QStringList outputQueue;

	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *debugMenu;
	QMenu *viewMenu;

	QToolBar *mainToolBar;

	QAction *quitAction;
	QAction *newFileAction;
	QAction *openFileAction;
	QAction *saveFileAction;
	QAction *saveAsAction;
	QAction *clearOutputAction;
	QAction *compileAction;
	QAction *runAction;

	jnc::Module module;
	jnc::Runtime runtime;
};

inline MainWindow* getMainWindow ()
{
	foreach (QWidget *widget, QApplication::topLevelWidgets())
	{
		MainWindow * mainWindow = qobject_cast<MainWindow *>(widget);
		if(mainWindow)
			return mainWindow;
	}

	return NULL;
}

#endif
