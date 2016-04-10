#include "pch.h"
#include "MyLayout.h"

//.............................................................................

MyLayout::MyLayout (QBoxLayout::Direction direction)
{
	m_qtLayout = new QBoxLayout (direction);
	m_direction = direction;
}

void 
MyLayout::markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap)
{
	size_t count = m_childWidgetList.count ();
	for (size_t i = 0; i < count; i++)
	{
		MyWidget* widget = m_childWidgetList [i];
		gcHeap->markClass (widget->m_box);
	}

	count = m_childLayoutList.count ();
	for (size_t i = 0; i < count; i++)
	{
		MyLayout* layout = m_childLayoutList [i];
		gcHeap->markClass (layout->m_box);
	}
}

void
AXL_CDECL
MyLayout::addWidget (MyWidget* widget)
{
	m_childWidgetList.append (widget);
	m_qtLayout->addWidget (widget->m_handle);
}

void
AXL_CDECL
MyLayout::addLayout (MyLayout* layout)
{
	m_childLayoutList.append (layout);
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
