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
#include "MyWidget.h"
#include "MyLib.h"

//..............................................................................

JNC_DEFINE_CLASS_TYPE(
	MyWidget,
	"Widget",
	g_myLibGuid,
	MyLibCacheSlot_Widget
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(MyWidget)
	JNC_MAP_AUTOGET_PROPERTY("m_isVisible", &MyWidget::setVisible)
	JNC_MAP_AUTOGET_PROPERTY("m_isEnabled", &MyWidget::setEnabled)
	JNC_MAP_FUNCTION("setSizePolicy", &MyWidget::setSizePolicy)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

MyWidget::MyWidget(QWidget* widget)
{
	m_handle = widget;

	QSizePolicy sizePolicy = widget->sizePolicy();
	m_hpolicy = sizePolicy.horizontalPolicy();
	m_vpolicy = sizePolicy.verticalPolicy();
	m_isVisible = widget->isVisible();
	m_isEnabled = widget->isEnabled();
}

void
JNC_CDECL
MyWidget::setSizePolicy(
	QSizePolicy::Policy hpolicy,
	QSizePolicy::Policy vpolicy
	)
{
	m_handle->setSizePolicy(hpolicy, vpolicy);
}

void
JNC_CDECL
MyWidget::setVisible(bool value)
{
	m_isVisible = value;
	m_handle->setVisible(value);
}

void
JNC_CDECL
MyWidget::setEnabled(bool value)
{
	m_isEnabled = value;
	m_handle->setEnabled(value);
}

//..............................................................................
