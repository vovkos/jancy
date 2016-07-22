#include "pch.h"
#include "MyTextEdit.h"
#include "MyLib.h"

//.............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	MyTextEdit,
	"TextEdit", 
	g_myLibGuid, 
	MyLibCacheSlot_TextEdit,
	MyTextEdit, 
	NULL)

JNC_BEGIN_TYPE_FUNCTION_MAP (MyTextEdit)
	JNC_MAP_CONSTRUCTOR (&sl::construct <MyTextEdit>)
	JNC_MAP_DESTRUCTOR (&sl::destruct <MyTextEdit>)
	JNC_MAP_PROPERTY ("m_text", &MyTextEdit::getText, &MyTextEdit::setText)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

MyTextEdit::MyTextEdit ():
	MyWidget (new QLineEdit)
{
	m_qtLineEdit = (QLineEdit*) m_handle;
}

//.............................................................................
