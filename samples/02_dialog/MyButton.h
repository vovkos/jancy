#pragma once

#include "MyWidget.h"

//.............................................................................

class MyButton: public MyWidget
{
public:
	JNC_BEGIN_OPAQUE_CLASS_TYPE (MyButton, "Button", ApiSlot_Button)
		JNC_CONSTRUCTOR (&(rtl::construct <MyButton, jnc::DataPtr>))
		JNC_AUTOGET_PROPERTY ("m_text", &MyButton::setText)
	JNC_END_CLASS_TYPE ()

public: 
	jnc::DataPtr m_text;
	jnc::ClassBox <jnc::Multicast> m_onClicked;

public:
	QPushButton* m_qtButton;	
	QtSignalBridge* m_onClickedBridge;

public:
	MyButton (jnc::DataPtr textPtr);

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtButton->setText ((const char*) textPtr.m_p);
	}
};

//.............................................................................
