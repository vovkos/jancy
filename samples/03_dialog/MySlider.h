#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MySlider)

//..............................................................................

class MySlider: public MyWidget
{
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
	JNC_CDECL
	setMinimum (int minimum)
	{
		m_minimum = minimum;
		m_qtSlider->setMinimum (minimum);
	}

	void
	JNC_CDECL
	setMaximum (int maximum)
	{
		m_maximum = maximum;
		m_qtSlider->setMaximum (maximum);
	}

	int
	JNC_CDECL
	getValue ()
	{
		return m_qtSlider->value ();
	}

	void
	JNC_CDECL
	setValue (int value)
	{
		m_qtSlider->setValue (value);
	}
};

//..............................................................................
