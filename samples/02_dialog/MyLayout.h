#pragma once

#include "MyWidget.h"

//.............................................................................

class MyLayout: public jnc::rt::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MyLayout, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("Layout", g_myLibCacheSlot, MyLibTypeCacheSlot_Layout)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <MyLayout, QBoxLayout::Direction>))
		JNC_MAP_FUNCTION ("addWidget", &MyLayout::addWidget)
		JNC_MAP_FUNCTION ("addLayout", &MyLayout::addLayout)
		JNC_MAP_FUNCTION ("addSpacer", &MyLayout::addSpacer)
	JNC_END_CLASS_TYPE_MAP ()

public: 
	QBoxLayout::Direction m_direction;
	
public:
	QBoxLayout* m_qtLayout;

public:
	MyLayout (QBoxLayout::Direction direction);

	void
	AXL_CDECL
	addWidget (MyWidget* widget);

	void
	AXL_CDECL
	addLayout (MyLayout* layout);

	void
	AXL_CDECL
	addSpacer ();
};

//.............................................................................
