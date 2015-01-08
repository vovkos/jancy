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
	int minimum,
	int maximum
	)
{
	jnc::ApiObjBox <MySlider>* slider = (jnc::ApiObjBox <MySlider>*) jnc::StdLib::gcAllocate (getApiType ());
	slider->prime ();	
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
