#pragma once

#include "MyWidget.h"

//.............................................................................

class MyButton: public MyWidget
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MyButton, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("Button", g_myLibCacheSlot, MyLibTypeCacheSlot_Button)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <MyButton, jnc::rt::DataPtr>))
		JNC_MAP_DESTRUCTOR (&sl::destruct <MyButton>)
		JNC_MAP_AUTOGET_PROPERTY ("m_text", &MyButton::setText)
	JNC_END_CLASS_TYPE_MAP ()

public: 
	jnc::rt::DataPtr m_text;
	jnc::rt::ClassBox <jnc::rt::Multicast> m_onClicked;

public:
	QPushButton* m_qtButton;	
	QtSignalBridge* m_onClickedBridge;

public:
	MyButton (jnc::rt::DataPtr textPtr);
	
	~MyButton ()
	{
		delete m_qtButton;
		delete m_onClickedBridge;
	}

	void
	AXL_CDECL
	setText (jnc::rt::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtButton->setText ((const char*) textPtr.m_p);
	}
};

//.............................................................................
