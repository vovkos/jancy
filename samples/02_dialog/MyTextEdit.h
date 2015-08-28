#pragma once

#include "MyWidget.h"

//.............................................................................

class MyTextEdit: public MyWidget
{
public:
	JNC_BEGIN_OPAQUE_CLASS_TYPE (MyTextEdit, "TextEdit", ApiSlot_TextEdit)
		JNC_CONSTRUCTOR (&rtl::construct <MyTextEdit>)
		JNC_PROPERTY ("m_text", &MyTextEdit::getText, &MyTextEdit::setText)
	JNC_END_CLASS_TYPE ()

public:
	QLineEdit* m_qtLineEdit;
	QtSignalBridge* m_onTextChangedBridge;

public:
	MyTextEdit ();

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
