#pragma once

#include "MyWidget.h"

//.............................................................................

class MyButton: public MyWidget
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MyButton, NULL)

	JNC_BEGIN_TYPE_FUNCTION_MAP ("Button", g_myLibCacheSlot, MyLibCacheSlot_Button)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <MyButton, jnc::DataPtr>))
		JNC_MAP_DESTRUCTOR (&sl::destruct <MyButton>)
		JNC_MAP_AUTOGET_PROPERTY ("m_text", &MyButton::setText)
	JNC_END_TYPE_FUNCTION_MAP ()

public: 
	jnc::DataPtr m_text;
	jnc::ClassBox <jnc::Multicast> m_onClicked;

public:
	QPushButton* m_qtButton;	
	QtSignalBridge* m_onClickedBridge;

public:
	MyButton (jnc::DataPtr textPtr);
	
	~MyButton ()
	{
		delete m_qtButton;
		delete m_onClickedBridge;
	}

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtButton->setText ((const char*) textPtr.m_p);
	}
};

//.............................................................................
