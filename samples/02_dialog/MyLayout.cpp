#include "pch.h"
#include "MyLayout.h"

//.............................................................................

void
MyLayout::construct (
	QBoxLayout::Direction direction,
	QWidget* parent
	)
{
	m_qtLayout = new QBoxLayout (direction, parent);
	m_direction = direction;
}

MyLayout*
MyLayout::operatorNew (
	jnc::ClassType* type,
	QBoxLayout::Direction direction
	)
{
	MyLayout* layout = (MyLayout*) jnc::StdLib::allocateClass (type);
	layout->construct (direction);
	return layout;
}

void
AXL_CDECL
MyLayout::addWidget (MyWidget* widget)
{
	m_qtLayout->addWidget (widget->m_handle);
}

void
AXL_CDECL
MyLayout::addLayout (MyLayout* layout)
{
	m_qtLayout->addLayout (layout->m_qtLayout);
}

void
AXL_CDECL
MyLayout::addSpacer ()
{
	QSizePolicy::Policy hpolicy = QSizePolicy::Minimum;
	QSizePolicy::Policy vpolicy = QSizePolicy::Minimum;

	switch (m_direction)
	{
	case QBoxLayout::LeftToRight:
	case QBoxLayout::RightToLeft:
		hpolicy = QSizePolicy::Expanding;
		break;

	case QBoxLayout::TopToBottom:
	case QBoxLayout::BottomToTop:
		vpolicy = QSizePolicy::Expanding;
		break;
	}

	QSpacerItem* item = new QSpacerItem (0, 0, hpolicy, vpolicy);
	m_qtLayout->addSpacerItem (item);
}

//.............................................................................
