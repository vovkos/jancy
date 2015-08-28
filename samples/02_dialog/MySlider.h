#pragma once

#include "MyWidget.h"

//.............................................................................

class MySlider: public MyWidget
{
public:
	JNC_BEGIN_OPAQUE_CLASS_TYPE (MySlider, "Slider", ApiSlot_Slider)
		JNC_CONSTRUCTOR (&(rtl::construct <MySlider, int, int>))
		JNC_AUTOGET_PROPERTY ("m_minimum", &MySlider::setMinimum)
		JNC_AUTOGET_PROPERTY ("m_maximum", &MySlider::setMaximum)
		JNC_PROPERTY ("m_value", &MySlider::getValue, &MySlider::setValue)
	JNC_END_CLASS_TYPE ()

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
