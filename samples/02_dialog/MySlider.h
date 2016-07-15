#pragma once

#include "MyWidget.h"

//.............................................................................

class MySlider: public MyWidget
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MySlider, NULL)
	
	JNC_BEGIN_TYPE_FUNCTION_MAP ("Slider", g_myLibCacheSlot, MyLibCacheSlot_Slider)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <MySlider, int, int>))
		JNC_MAP_DESTRUCTOR (&sl::destruct <MySlider>)
		JNC_MAP_AUTOGET_PROPERTY ("m_minimum", &MySlider::setMinimum)
		JNC_MAP_AUTOGET_PROPERTY ("m_maximum", &MySlider::setMaximum)
		JNC_MAP_PROPERTY ("m_value", &MySlider::getValue, &MySlider::setValue)
	JNC_END_TYPE_FUNCTION_MAP ()

public: 
	int m_minimum;
	int m_maximum;
	
	jnc::ClassBox <jnc::Multicast> m_onValueChanged;
	
public:
	QSlider* m_qtSlider;
	QtSignalBridge* m_onValueChangedBridge;

public:
	MySlider (
		int minimum,
		int maximum
		);

	~MySlider ()
	{
		delete m_qtSlider;
		delete m_onValueChangedBridge;
	}

	void
	AXL_CDECL
	setMinimum (int minimum)
	{
		m_minimum = minimum;
		m_qtSlider->setMinimum (minimum);
	}

	void
	AXL_CDECL
	setMaximum (int maximum)
	{
		m_maximum = maximum;
		m_qtSlider->setMaximum (maximum);
	}

	int
	AXL_CDECL
	getValue ()
	{
		return m_qtSlider->value ();
	}

	void
	AXL_CDECL
	setValue (int value)
	{
		m_qtSlider->setValue (value);
	}
};

//.............................................................................
