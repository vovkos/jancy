#pragma once

#include "MyWidget.h"

//.............................................................................

class MyLayout: public jnc::IfaceHdr
{
public:
	JNC_BEGIN_OPAQUE_CLASS_TYPE (MyLayout, "Layout", ApiSlot_Layout)
		JNC_CONSTRUCTOR (&(rtl::construct <MyLayout, QBoxLayout::Direction>))
		JNC_FUNCTION ("addWidget", &MyLayout::addWidget)
		JNC_FUNCTION ("addLayout", &MyLayout::addLayout)
		JNC_FUNCTION ("addSpacer", &MyLayout::addSpacer)
	JNC_END_CLASS_TYPE ()

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
