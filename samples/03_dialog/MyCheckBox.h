#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MyCheckBox)

//.............................................................................

class MyCheckBox: public MyWidget
{
public: 
	jnc::DataPtr m_text;
	jnc::ClassBox <jnc::Multicast> m_onIsCheckedChanged;
	
public:
	QCheckBox* m_qtCheckBox;
	QtSignalBridge* m_onIsCheckedChangedBridge;

public:
	MyCheckBox (jnc::DataPtr textPtr);
	
	~MyCheckBox ()
	{
		delete m_qtCheckBox;
		delete m_onIsCheckedChangedBridge;
	}

	void
	JNC_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtCheckBox->setText ((const char*) textPtr.m_p);
	}

	bool
	JNC_CDECL
	isChecked ()
	{
		return m_qtCheckBox->isChecked ();
	}

	void
	JNC_CDECL
	setChecked (bool value)
	{
		m_qtCheckBox->setChecked (value);
	}
};

//.............................................................................
