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
#include "MyLayout.h"
#include "MyLib.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	MyLayout,
	"Layout",
	g_myLibGuid,
	MyLibCacheSlot_Layout,
	MyLayout,
	&MyLayout::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (MyLayout)
	JNC_MAP_CONSTRUCTOR (&(jnc::construct <MyLayout, QBoxLayout::Direction>))
	JNC_MAP_DESTRUCTOR (&jnc::destruct <MyLayout>)
	JNC_MAP_FUNCTION ("addWidget", &MyLayout::addWidget)
	JNC_MAP_FUNCTION ("addLayout", &MyLayout::addLayout)
	JNC_MAP_FUNCTION ("addSpacer", &MyLayout::addSpacer)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

MyLayout::MyLayout (QBoxLayout::Direction direction)
{
	m_qtLayout = new QBoxLayout (direction);
	m_direction = direction;
}

void
MyLayout::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	int count = m_childWidgetList.count ();
	for (int i = 0; i < count; i++)
	{
		MyWidget* widget = m_childWidgetList [i];
		gcHeap->markClass (widget->m_box);
	}

	count = m_childLayoutList.count ();
	for (int i = 0; i < count; i++)
	{
		MyLayout* layout = m_childLayoutList [i];
		gcHeap->markClass (layout->m_box);
	}
}

void
JNC_CDECL
MyLayout::addWidget (MyWidget* widget)
{
	m_childWidgetList.append (widget);
	m_qtLayout->addWidget (widget->m_handle);
}

void
JNC_CDECL
MyLayout::addLayout (MyLayout* layout)
{
	m_childLayoutList.append (layout);
	m_qtLayout->addLayout (layout->m_qtLayout);
}

void
JNC_CDECL
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

//..............................................................................
