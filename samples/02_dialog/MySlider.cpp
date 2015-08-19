#include "pch.h"
#include "MySlider.h"

//.............................................................................

MySlider::MySlider (
	int minimum,
	int maximum
	):
	MyWidget (new QSlider)
{
	m_qtSlider = (QSlider*) m_handle;
	m_qtSlider->setOrientation (Qt::Horizontal);
	setMinimum (minimum);
	setMaximum (maximum);

	m_onValueChangedBridge = new QtSignalBridge;
	m_onValueChangedBridge->connect (m_qtSlider, SIGNAL (valueChanged (int)), &m_onValueChanged);
}

//.............................................................................
