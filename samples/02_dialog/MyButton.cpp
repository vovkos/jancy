#include "pch.h"
#include "MyButton.h"
#include "QtSignalBridge.h"

//.............................................................................

void 
MyButton::construct (jnc::DataPtr textPtr)
{
	m_qtButton = new QPushButton;
	MyWidget::construct (m_qtButton);
	setText (textPtr);
	m_onClickedBridge = new QtSignalBridge;
	m_onClickedBridge->connect (m_qtButton, SIGNAL (clicked ()), &m_onClicked);
}

MyButton*
MyButton::operatorNew (
	jnc::ClassType* type,
	jnc::DataPtr textPtr
	)
{
	MyButton* button = (MyButton*) jnc::StdLib::allocateClass (type);
	button->construct (textPtr);
	return button;
}

//.............................................................................
