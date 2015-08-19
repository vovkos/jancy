#include "pch.h"
#include "MyCheckBox.h"

//.............................................................................

MyCheckBox::MyCheckBox (jnc::DataPtr textPtr):
	MyWidget (new QCheckBox)
{
	m_qtCheckBox = (QCheckBox*) m_handle;
	setText (textPtr);

	m_onIsCheckedChangedBridge = new QtSignalBridge;
	m_onIsCheckedChangedBridge->connect (m_qtCheckBox, SIGNAL (stateChanged (int)), &m_onIsCheckedChanged);
}

//.............................................................................
