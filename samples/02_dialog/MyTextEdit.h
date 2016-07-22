#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MyTextEdit)

//.............................................................................

class MyTextEdit: public MyWidget
{
public:
	QLineEdit* m_qtLineEdit;
	QtSignalBridge* m_onTextChangedBridge;

public:
	MyTextEdit ();

	~MyTextEdit ()
	{
		delete m_qtLineEdit;
		delete m_onTextChangedBridge;
	}

	static
	jnc::DataPtr
	AXL_CDECL
	getText (MyTextEdit* self)
	{
		QByteArray text = self->m_qtLineEdit->text ().toUtf8 ();
		return jnc::strDup (text, text.length ());
	}

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_qtLineEdit->setText ((const char*) textPtr.m_p);
	}
};

//.............................................................................
