#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (MyLayout)

//.............................................................................

class MyLayout: public jnc::IfaceHdr
{
public: 
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (MyLayout)

	QBoxLayout::Direction m_direction;
	
public:
	QBoxLayout* m_qtLayout;	

protected:
	QList <MyWidget*> m_childWidgetList;
	QList <MyLayout*> m_childLayoutList;

public:
	MyLayout (QBoxLayout::Direction direction);
	
	~MyLayout ()
	{
		delete m_qtLayout;
	}

	void 
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

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
