#include "pch.h"
#include "MySlider.h"

//.............................................................................

void
MySlider::construct (
	int minimum,
	int maximum
	)
{
	m_qtSlider = new QSlider;
	m_qtSlider->setOrientation (Qt::Horizontal);
	MyWidget::construct (m_qtSlider);
	setMinimum (minimum);
	setMaximum (maximum);

	m_onValueChangedBridge = new QtSignalBridge;
	m_onValueChangedBridge->connect (m_qtSlider, SIGNAL (valueChanged (int)), &m_onValueChanged);
}

MySlider*
MySlider::operatorNew (
	jnc::ClassType* type,
	int minimum,
	int maximum
	)
{
	MySlider* slider = (MySlider*) jnc::StdLib::allocateClass (type);
	slider->construct (minimum, maximum);
	return slider;
}

//.............................................................................
