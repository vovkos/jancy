#include "pch.h"
#include "MyWidget.h"

//.............................................................................

MyWidget::MyWidget (QWidget* widget)
{
	m_handle = widget;

	QSizePolicy sizePolicy = widget->sizePolicy ();
	m_hpolicy = sizePolicy.horizontalPolicy ();
	m_vpolicy = sizePolicy.verticalPolicy ();
	m_isVisible = widget->isVisible ();
	m_isEnabled = widget->isEnabled ();
}

void
AXL_CDECL
MyWidget::setSizePolicy (
	QSizePolicy::Policy hpolicy,
	QSizePolicy::Policy vpolicy
	)
{
	m_handle->setSizePolicy (hpolicy, vpolicy);
}

void
AXL_CDECL
MyWidget::setVisible (bool value)
{
	m_isVisible = value;
	m_handle->setVisible (value);
}

void
AXL_CDECL
MyWidget::setEnabled (bool value)
{
	m_isEnabled = value;
	m_handle->setEnabled (value);
}

//.............................................................................
