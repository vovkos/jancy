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
#include "MyButton.h"
#include "MyLib.h"
#include "QtSignalBridge.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	MyButton,
	"Button",
	g_myLibGuid,
	MyLibCacheSlot_Button,
	MyButton,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(MyButton)
	JNC_MAP_CONSTRUCTOR(&(jnc::construct<MyButton, jnc::DataPtr>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<MyButton>)
	JNC_MAP_AUTOGET_PROPERTY("m_text", &MyButton::setText)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

MyButton::MyButton(jnc::DataPtr textPtr):
	MyWidget(new QPushButton) {
	m_qtButton = (QPushButton*)m_handle;
	setText(textPtr);
	m_onClickedBridge = new QtSignalBridge;
	m_onClickedBridge->connect(m_qtButton, SIGNAL(clicked()), m_onClicked);
}

MyButton::~MyButton() {
	if (!m_qtButton->parent())
		delete m_qtButton;

	delete m_onClickedBridge;
}


//..............................................................................
