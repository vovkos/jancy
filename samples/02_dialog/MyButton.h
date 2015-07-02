#pragma once

#include "MyWidget.h"

//.............................................................................

class MyButton: public MyWidget
{
public:
	JNC_BEGIN_CLASS ("Button", ApiSlot_Button)
		JNC_OPAQUE_CLASS (MyButton, &enumGcRoots)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_AUTOGET_PROPERTY ("m_text", &MyButton::setText)
	JNC_END_CLASS ()

public: 
	jnc::DataPtr m_text;
	jnc::ClassBox <jnc::Multicast> m_onClicked;

public:
	QPushButton* m_qtButton;	
	QtSignalBridge* m_onClickedBridge;

public:
	static
	void
	enumGcRoots (
		jnc::Runtime* runtime,
		MyButton* self
		);

	static 
	MyButton*
	operatorNew (
		jnc::ClassType* type,
		jnc::DataPtr textPtr
		);

	void
	construct (jnc::DataPtr textPtr);

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtButton->setText ((const char*) textPtr.m_p);
	}
};

//.............................................................................
