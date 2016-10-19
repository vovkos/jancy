#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MyLabel)

//..............................................................................

class MyLabel: public MyWidget
{
public:
	jnc::DataPtr m_text;
	int m_color;
	int m_backColor;
	Qt::Alignment m_alignment;

public:
	QLabel* m_qtLabel;

public:
	MyLabel (jnc::DataPtr textPtr);

	~MyLabel ()
	{
		delete m_qtLabel;
	}

	void
	JNC_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtLabel->setText ((const char*) textPtr.m_p);
	}

	void
	JNC_CDECL
	setColor (int color)
	{
		m_color = color;
		updateStyleSheet ();
	}

	void
	JNC_CDECL
	setBackColor (int color)
	{
		m_backColor = color;
		updateStyleSheet ();
	}

	void
	JNC_CDECL
	setAlignment (Qt::Alignment alignment)
	{
		m_alignment = alignment;
		m_qtLabel->setAlignment (alignment);
	}

protected:
	void
	updateStyleSheet ();
};

//..............................................................................
