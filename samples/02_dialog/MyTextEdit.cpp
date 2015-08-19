#include "pch.h"
#include "MyTextEdit.h"

//.............................................................................

MyTextEdit::MyTextEdit ():
	MyWidget (new QLineEdit)
{
	m_qtLineEdit = (QLineEdit*) m_handle;
}

//.............................................................................
