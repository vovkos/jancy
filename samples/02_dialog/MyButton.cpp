#include "pch.h"
#include "MyButton.h"
#include "QtSignalBridge.h"

//.............................................................................

void
MyButton::enumGcRoots (
	jnc::Runtime* runtime,
	MyButton* self
	)
{
}

MyButton*
MyButton::operatorNew (jnc::DataPtr textPtr)
{
	jnc::ApiObjBox <MyButton>* button = (jnc::ApiObjBox <MyButton>*) jnc::StdLib::gcAllocate (getApiType ());
	button->prime ();	
	button->construct (textPtr);
	return button;
}

void 
MyButton::construct (jnc::DataPtr textPtr)
{
	m_qtButton = new QPushButton;
	MyWidget::construct (m_qtButton);
	setText (textPtr);
	m_onClickedBridge = new QtSignalBridge;
	m_onClickedBridge->connect (m_qtButton, SIGNAL (clicked ()), &m_onClicked);
}

//.............................................................................
