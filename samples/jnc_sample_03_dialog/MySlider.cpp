//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "MySlider.h"
#include "MyLib.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	MySlider,
	"Slider",
	g_myLibGuid,
	MyLibCacheSlot_Slider,
	MySlider,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(MySlider)
	JNC_MAP_CONSTRUCTOR(&(jnc::construct<MySlider, int, int>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<MySlider>)
	JNC_MAP_AUTOGET_PROPERTY("m_minimum", &MySlider::setMinimum)
	JNC_MAP_AUTOGET_PROPERTY("m_maximum", &MySlider::setMaximum)
	JNC_MAP_PROPERTY("m_value", &MySlider::getValue, &MySlider::setValue)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

MySlider::MySlider(
	int minimum,
	int maximum
	):
	MyWidget(new QSlider)
{
	m_qtSlider = (QSlider*)m_handle;
	m_qtSlider->setOrientation(Qt::Horizontal);
	setMinimum(minimum);
	setMaximum(maximum);

	m_onValueChangedBridge = new QtSignalBridge;
	m_onValueChangedBridge->connect(m_qtSlider, SIGNAL(valueChanged(int)), m_onValueChanged);
}

MySlider::~MySlider()
{
	if (!m_qtSlider)
		delete m_qtSlider;

	delete m_onValueChangedBridge;
}

//..............................................................................
