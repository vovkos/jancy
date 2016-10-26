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

#include "MyLayout.h"

//..............................................................................

class MainWindow : public QMainWindow
{
	Q_OBJECT

protected:
	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;
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

	QString readOutput ()
	{
		return m_output->toPlainText ();
	}

protected:
	virtual void closeEvent (QCloseEvent* e);

	void createLayout ();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

MainWindow* getMainWindow ();

//..............................................................................
