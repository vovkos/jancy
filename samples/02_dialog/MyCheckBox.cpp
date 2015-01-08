#include "pch.h"
#include "MyCheckBox.h"

//.............................................................................

void
MyCheckBox::enumGcRoots (
	jnc::Runtime* runtime,
	MyCheckBox* self
	)
{
}

MyCheckBox*
MyCheckBox::operatorNew (jnc::DataPtr textPtr)
{
	jnc::ApiObjBox <MyCheckBox>* checkBox = (jnc::ApiObjBox <MyCheckBox>*) jnc::StdLib::gcAllocate (getApiType ());
	checkBox->prime ();	
	checkBox->construct (textPtr);
	return checkBox;
}

void
MyCheckBox::construct (jnc::DataPtr textPtr)
{
	m_qtCheckBox = new QCheckBox;
	MyWidget::construct (m_qtCheckBox);
	setText (textPtr);

	m_onIsCheckedChangedBridge = new QtSignalBridge;
	m_onIsCheckedChangedBridge->connect (m_qtCheckBox, SIGNAL (stateChanged (int)), &m_onIsCheckedChanged);
}

//.............................................................................
