#pragma once

#include "MyWidget.h"

//.............................................................................

class MyCheckBox: public MyWidget
{
public:
	JNC_BEGIN_CLASS ("CheckBox", ApiSlot_CheckBox)
		JNC_OPAQUE_CLASS (MyCheckBox, NULL)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_AUTOGET_PROPERTY ("m_text", &MyCheckBox::setText)
		JNC_PROPERTY ("m_isChecked", &MyCheckBox::isChecked, &MyCheckBox::setChecked)
	JNC_END_CLASS ()

public: 
	jnc::DataPtr m_text;
	jnc::ClassBox <jnc::Multicast> m_onIsCheckedChanged;
	
public:
	QCheckBox* m_qtCheckBox;
	QtSignalBridge* m_onIsCheckedChangedBridge;

public:
	void
	construct (jnc::DataPtr textPtr);

	static 
	MyCheckBox*
	operatorNew (
		jnc::ClassType* type,
		jnc::DataPtr textPtr
		);

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtCheckBox->setText ((const char*) textPtr.m_p);
	}

	bool
	AXL_CDECL
	isChecked ()
	{
		return m_qtCheckBox->isChecked ();
	}

	void
	AXL_CDECL
	setChecked (bool value)
	{
		m_qtCheckBox->setChecked (value);
	}
};

//.............................................................................
