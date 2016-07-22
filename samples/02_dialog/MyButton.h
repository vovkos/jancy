#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MyButton)

//.............................................................................

class MyButton: public MyWidget
{
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
