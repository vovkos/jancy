#include "pch.h"
#include "MyCheckBox.h"
#include "MyLib.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	MyCheckBox,
	"CheckBox",
	g_myLibGuid,
	MyLibCacheSlot_CheckBox,
	MyCheckBox,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (MyCheckBox)
	JNC_MAP_CONSTRUCTOR (&(jnc::construct <MyCheckBox, jnc::DataPtr>))
	JNC_MAP_DESTRUCTOR (&jnc::destruct <MyCheckBox>)
	JNC_MAP_AUTOGET_PROPERTY ("m_text", &MyCheckBox::setText)
	JNC_MAP_PROPERTY ("m_isChecked", &MyCheckBox::isChecked, &MyCheckBox::setChecked)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

MyCheckBox::MyCheckBox (jnc::DataPtr textPtr):
	MyWidget (new QCheckBox)
{
	m_qtCheckBox = (QCheckBox*) m_handle;
	setText (textPtr);

	m_onIsCheckedChangedBridge = new QtSignalBridge;
	m_onIsCheckedChangedBridge->connect (m_qtCheckBox, SIGNAL (stateChanged (int)), m_onIsCheckedChanged);
}

//..............................................................................
