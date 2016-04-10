#pragma once

#include "MyWidget.h"

//.............................................................................

class MyLabel: public MyWidget
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MyLabel, NULL)
	
	JNC_BEGIN_CLASS_TYPE_MAP ("Label", g_myLibCacheSlot, MyLibTypeCacheSlot_Label)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <MyLabel, jnc::rt::DataPtr>))
		JNC_MAP_DESTRUCTOR (&sl::destruct <MyLabel>)
		JNC_MAP_AUTOGET_PROPERTY ("m_text", &MyLabel::setText)
		JNC_MAP_AUTOGET_PROPERTY ("m_color", &MyLabel::setColor)
		JNC_MAP_AUTOGET_PROPERTY ("m_backColor", &MyLabel::setBackColor)
		JNC_MAP_AUTOGET_PROPERTY ("m_alignment", &MyLabel::setAlignment)
	JNC_END_CLASS_TYPE_MAP ()

public: 
	jnc::rt::DataPtr m_text;
	int m_color;
	int m_backColor;
	Qt::Alignment m_alignment;
	
public:
	QLabel* m_qtLabel;

public:
	MyLabel (jnc::rt::DataPtr textPtr);
	
	~MyLabel ()
	{
		delete m_qtLabel;
	}

	void
	AXL_CDECL
	setText (jnc::rt::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtLabel->setText ((const char*) textPtr.m_p);
	}

	void
	AXL_CDECL
	setColor (int color)
	{	
		m_color = color;
		updateStyleSheet ();
	}

	void
	AXL_CDECL
	setBackColor (int color)
	{	
		m_backColor = color;
		updateStyleSheet ();
	}

	void
	AXL_CDECL
	setAlignment (Qt::Alignment alignment)
	{	
		m_alignment = alignment;
		m_qtLabel->setAlignment (alignment);
	}

protected:
	void
	updateStyleSheet ();
};

//.............................................................................
