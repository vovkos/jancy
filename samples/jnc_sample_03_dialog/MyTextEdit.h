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

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MyTextEdit)

//..............................................................................

class MyTextEdit: public MyWidget
{
public:
	QLineEdit* m_qtLineEdit;
	QtSignalBridge* m_onTextChangedBridge;

public:
	MyTextEdit ();
	~MyTextEdit ();

	static
	jnc::DataPtr
	JNC_CDECL
	getText (MyTextEdit* self)
	{
		QByteArray text = self->m_qtLineEdit->text ().toUtf8 ();
		return jnc::strDup (text, text.length ());
	}

	void
	JNC_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_qtLineEdit->setText ((const char*) textPtr.m_p);
	}
};

//..............................................................................
