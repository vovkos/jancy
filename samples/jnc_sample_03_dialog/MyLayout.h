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

#pragma once

#include "MyWidget.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE(MyLayout)

//..............................................................................

class MyLayout: public jnc::IfaceHdr
{
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(MyLayout)

	QBoxLayout::Direction m_direction;

public:
	QBoxLayout* m_qtLayout;

protected:
	QList<MyWidget*> m_childWidgetList;
	QList<MyLayout*> m_childLayoutList;

public:
	MyLayout(QBoxLayout::Direction direction);
	~MyLayout();

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	void
	JNC_CDECL
	addWidget(MyWidget* widget);

	void
	JNC_CDECL
	addLayout(MyLayout* layout);

	void
	JNC_CDECL
	addSpacer();
};

//..............................................................................
