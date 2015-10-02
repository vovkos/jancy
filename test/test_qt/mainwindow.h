#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

class TestClassA: public jnc::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("TestClassA", -1, -1)
		JNC_MAP_FUNCTION ("foo", &TestClassA::foo)
	JNC_END_CLASS_TYPE_MAP ()

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

class TestClassB: public jnc::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (TestClassB, &TestClassB::markOpaqueGcRoots)

	JNC_BEGIN_CLASS_TYPE_MAP ("TestClassB", -1, -1)
		JNC_MAP_FUNCTION ("bar", &TestClassB::bar)
	JNC_END_CLASS_TYPE_MAP ()

public:
	char m_data [256];

public:
	void
	AXL_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	bool
	AXL_CDECL
	bar (
		jnc::DataPtr ptr1,
		jnc::DataPtr ptr2,
		jnc::DataPtr ptr3,
		jnc::DataPtr ptr4,
		int a,
		int b
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestStruct
{
public:
	JNC_BEGIN_TYPE_MAP ("TestStruct", -1, -1)
		JNC_MAP_CONSTRUCTOR (&TestStruct::construct_0)
		JNC_MAP_OVERLOAD (&TestStruct::construct_1)
		JNC_MAP_OVERLOAD (&TestStruct::construct_2)

		JNC_MAP_FUNCTION ("foo", &TestStruct::foo_0)
		JNC_MAP_OVERLOAD (&TestStruct::foo_1)
		JNC_MAP_OVERLOAD (&TestStruct::foo_2)
	JNC_END_TYPE_MAP ()

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

class TestLib: public jnc::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("printf", &TestLib::printf)
//		JNC_MAP_TYPE (TestClassA)
//		JNC_MAP_TYPE (TestClassB)
//		JNC_MAP_TYPE (TestStruct)
//		JNC_MAP_FUNCTION ("testPtr",     &testPtr)
//		JNC_MAP_FUNCTION ("testVariant", &testVariant)
//		JNC_MAP_FUNCTION ("qtWait", &qtWait)
	JNC_END_LIB_MAP ()
	
	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY ("TestClassB", TestClassB)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()
	
	static
	int
	printf (
		const char* format,
		...
		);

	static
	void
	testPtr (jnc::DataPtr ptr);

	static
	void
	testVariant (jnc::Variant variant);

	static
	void
	qtWait (uint_t msTime);
};

//.............................................................................

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	QSize sizeHint() const { return QSize(800, 600); }

	void writeStatus(const QString &text, int timeout = 0);
	
	size_t writeOutputDirect(const char* text, size_t length = -1);
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
	void outputSlot ();

signals:
	void outputSignal ();

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

	jnc::Module m_module;
	jnc::Runtime m_runtime;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline MainWindow* getMainWindow ()
{
	extern MainWindow* g_mainWindow;
	return g_mainWindow;
}

//.............................................................................

#endif
