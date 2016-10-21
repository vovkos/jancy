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

#include "MyLibGlobals.h"
#include "QtSignalBridge.h"

JNC_DECLARE_CLASS_TYPE (MyWidget)

//..............................................................................

class MyWidget: public jnc::IfaceHdr
{
public:
	QWidget* m_handle;
	QSizePolicy::Policy m_hpolicy;
	QSizePolicy::Policy m_vpolicy;
	bool m_isVisible;
	bool m_isEnabled;

public:
	MyWidget (QWidget* widget);

	void
	JNC_CDECL
	setSizePolicy (
		QSizePolicy::Policy hpolicy,
		QSizePolicy::Policy vpolicy
		);

	void
	JNC_CDECL
	setVisible (bool value);

	void
	JNC_CDECL
	setEnabled (bool value);
};

//..............................................................................
