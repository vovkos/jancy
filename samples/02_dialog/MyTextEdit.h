#pragma once

#include "MyWidget.h"

//.............................................................................

class MyTextEdit: public MyWidget
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MyTextEdit, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("TextEdit", g_myLibCacheSlot, MyLibTypeCacheSlot_TextEdit)
		JNC_MAP_CONSTRUCTOR (&sl::construct <MyTextEdit>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <MyTextEdit>)
		JNC_MAP_PROPERTY ("m_text", &MyTextEdit::getText, &MyTextEdit::setText)
	JNC_END_CLASS_TYPE_MAP ()

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
		return jnc::rt::strDup (text, text.length ());
	}

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_qtLineEdit->setText ((const char*) textPtr.m_p);
	}
};

//.............................................................................
