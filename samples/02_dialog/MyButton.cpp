#include "pch.h"
#include "MyButton.h"
#include "QtSignalBridge.h"

//.............................................................................

MyButton::MyButton (jnc::DataPtr textPtr):
	MyWidget (new QPushButton)
{
	m_qtButton = (QPushButton*) m_handle;
	setText (textPtr);
	m_onClickedBridge = new QtSignalBridge;
	m_onClickedBridge->connect (m_qtButton, SIGNAL (clicked ()), m_onClicked);
}

//.............................................................................
