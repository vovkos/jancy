#pragma once

#include "MyLayout.h"

//.............................................................................

class MainWindow : public QMainWindow
{
	Q_OBJECT

protected:
	jnc::ct::Module m_module;
	jnc::rt::Runtime m_runtime;
	QWidget* m_body;
	QPlainTextEdit* m_output;
	MyLayout* m_layout;

public:
	MainWindow (QWidget* parent = NULL, Qt::WindowFlags flags = 0);
	
	QSize sizeHint() const 
	{ 
		return QSize(800, 600); 
	}

	bool runScript (const QString& fileName);

	int output_va (const char* format, va_list va);

	int output (const char* format, ...)
	{
		va_list va;
		va_start (va, format);
		return output_va (format, va);
	}

protected:
	virtual void closeEvent (QCloseEvent* e);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

MainWindow* getMainWindow ();

//.............................................................................

