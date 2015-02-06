#pragma once

#include "MyWidget.h"

//.............................................................................

class MyTextEdit: public MyWidget
{
public:
	JNC_BEGIN_CLASS ("TextEdit", ApiSlot_TextEdit)
		JNC_OPAQUE_CLASS (MyTextEdit, &enumGcRoots)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_PROPERTY ("m_text", &MyTextEdit::getText, &MyTextEdit::setText)
	JNC_END_CLASS ()

public:
	QLineEdit* m_qtLineEdit;
	QtSignalBridge* m_onTextChangedBridge;

public:
	static
	void
	enumGcRoots (
		jnc::Runtime* runtime,
		MyTextEdit* self
		);

	static 
	MyTextEdit*
	operatorNew (jnc::ClassType* type);

	void
	construct ();

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
