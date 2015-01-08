#include "pch.h"
#include "MyLayout.h"

//.............................................................................

void
MyLayout::enumGcRoots (
	jnc::Runtime* runtime,
	MyLayout* self
	)
{
}

MyLayout*
MyLayout::operatorNew (QBoxLayout::Direction direction)
{
	jnc::ApiObjBox <MyLayout>* layout = (jnc::ApiObjBox <MyLayout>*) jnc::StdLib::gcAllocate (getApiType ());
	layout->prime ();	
	layout->construct (direction);
	return layout;
}

void
MyLayout::construct (
	QBoxLayout::Direction direction,
	QWidget* parent
	)
{
	m_qtLayout = new QBoxLayout (direction, parent);
	m_direction = direction;
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
