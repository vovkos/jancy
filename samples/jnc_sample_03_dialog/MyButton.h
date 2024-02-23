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

JNC_DECLARE_OPAQUE_CLASS_TYPE(MyButton)

//..............................................................................

class MyButton: public MyWidget {
public:
	jnc::DataPtr m_text;
	jnc::ClassBox<jnc::Multicast> m_onClicked;

public:
	QPushButton* m_qtButton;
	QtSignalBridge* m_onClickedBridge;

public:
	MyButton(jnc::DataPtr textPtr);
	~MyButton();

	void
	JNC_CDECL
	setText(jnc::DataPtr textPtr) {
		m_text = textPtr;
		m_qtButton->setText((const char*)textPtr.m_p);
	}
};

//..............................................................................
