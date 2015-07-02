#include "pch.h"
#include "MySlider.h"

//.............................................................................

void
MySlider::enumGcRoots (
	jnc::Runtime* runtime,
	MySlider* self
	)
{
}

MySlider*
MySlider::operatorNew (
	jnc::ClassType* type,
	int minimum,
	int maximum
	)
{
	jnc::ApiClassBox <MySlider>* slider = (jnc::ApiClassBox <MySlider>*) jnc::StdLib::gcAllocate (type);
	slider->prime (type);	
	slider->construct (minimum, maximum);
	return slider;
}

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

//.............................................................................
