#pragma once

#include "MyWidget.h"

//.............................................................................

class MyCheckBox: public MyWidget
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MyCheckBox, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("CheckBox", g_myLibCacheSlot, MyLibTypeCacheSlot_CheckBox)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <MyCheckBox, jnc::rt::DataPtr>))
		JNC_MAP_AUTOGET_PROPERTY ("m_text", &MyCheckBox::setText)
		JNC_MAP_PROPERTY ("m_isChecked", &MyCheckBox::isChecked, &MyCheckBox::setChecked)
	JNC_END_CLASS_TYPE_MAP ()

public: 
	jnc::rt::DataPtr m_text;
	jnc::rt::ClassBox <jnc::rt::Multicast> m_onIsCheckedChanged;
	
public:
	QCheckBox* m_qtCheckBox;
	QtSignalBridge* m_onIsCheckedChangedBridge;

public:
	MyCheckBox (jnc::rt::DataPtr textPtr);

	void
	AXL_CDECL
	setText (jnc::rt::DataPtr textPtr)
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
