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
	ApiSlot_TestClass = 0,
	ApiSlot_TestStruct,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestClass: public jnc::IfaceHdr
{
public:
	JNC_API_BEGIN_CLASS ("TestClass", ApiSlot_TestClass)
		JNC_API_CONSTRUCTOR (&TestClass::construct_0)
		JNC_API_OVERLOAD (&TestClass::construct_1)
		JNC_API_OVERLOAD (&TestClass::construct_2)

		JNC_API_FUNCTION ("foo", &TestClass::foo_0)
		JNC_API_OVERLOAD (&TestClass::foo_1)
		JNC_API_OVERLOAD (&TestClass::foo_2)
	JNC_API_END_CLASS ()

public:
	int m_x;
	double m_y;

public:
	void
	AXL_CDECL
	construct_0 ();

	void
	AXL_CDECL
	construct_1 (int x);

	void
	AXL_CDECL
	construct_2 (double y);

	void
	AXL_CDECL
	foo_0 ();

	void
	AXL_CDECL
	foo_1 (int x);

	void
	AXL_CDECL
	foo_2 (double y);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestStruct
{
public:
	JNC_API_BEGIN_TYPE ("TestStruct", ApiSlot_TestStruct)
		JNC_API_CONSTRUCTOR (&TestStruct::construct_0)
		JNC_API_OVERLOAD (&TestStruct::construct_1)
		JNC_API_OVERLOAD (&TestStruct::construct_2)

		JNC_API_FUNCTION ("foo", &TestStruct::foo_0)
		JNC_API_OVERLOAD (&TestStruct::foo_1)
		JNC_API_OVERLOAD (&TestStruct::foo_2)
	JNC_API_END_TYPE ()

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
	JNC_API_BEGIN_LIB ()
		JNC_API_STD_FUNCTION (jnc::StdFunction_Printf,  &Printf)
//		JNC_API_TYPE (TestClass)
//		JNC_API_TYPE (TestStruct)
		JNC_API_LIB (jnc::StdLib)
//		JNC_API_FUNCTION ("testPtr",  &testPtr)
	JNC_API_END_LIB ()

	static
	int
	Printf (
		const char* pFormat,
		...
		);

	static
	void
	testPtr (int x, jnc::DataPtr Ptr, int y);
};

//.............................................................................

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	QSize sizeHint() const { return QSize(800, 600); }

	void writeStatus(const QString &text, int timeout = 0);
	void writeOutput_va(const char* format, va_list va);
	void writeOutput(const char* format, ...);
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

inline MainWindow *GetMainWindow()
{
	foreach (QWidget *widget, QApplication::topLevelWidgets())
	{
		MainWindow * mainWindow = qobject_cast<MainWindow *>(widget);
		if(mainWindow)
			return mainWindow;
	}

	return NULL;
}

inline void WriteOutput_va(const char* format, va_list va)
{
	GetMainWindow()->writeOutput_va(format, va);
}

inline void WriteOutput(const char* format, ...)
{
	va_list va;
	va_start (va, format);
	GetMainWindow()->writeOutput_va(format, va);
}

#endif
