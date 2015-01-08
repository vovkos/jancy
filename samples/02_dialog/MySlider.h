#pragma once

#include "MyWidget.h"

//.............................................................................

class MySlider: public MyWidget
{
public:
	JNC_BEGIN_CLASS ("Slider", ApiSlot_Slider)
		JNC_OPAQUE_CLASS (MySlider, &enumGcRoots)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_AUTOGET_PROPERTY ("m_minimum", &MySlider::setMinimum)
		JNC_AUTOGET_PROPERTY ("m_maximum", &MySlider::setMaximum)
		JNC_PROPERTY ("m_value", &MySlider::getValue, &MySlider::setValue)
	JNC_END_CLASS ()

public: 
	int m_minimum;
	int m_maximum;
	
	jnc::ObjBox <jnc::Multicast> m_onValueChanged;
	
public:
	QSlider* m_qtSlider;
	QtSignalBridge* m_onValueChangedBridge;

public:
	static
	void
	enumGcRoots (
		jnc::Runtime* runtime,
		MySlider* self
		);

	static 
	MySlider*
	operatorNew (
		int minimum,
		int maximum
		);

	void
	construct (
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
