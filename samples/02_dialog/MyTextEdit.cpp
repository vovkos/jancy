#include "pch.h"
#include "MyTextEdit.h"

//.............................................................................

void
MyTextEdit::enumGcRoots (
	jnc::Runtime* runtime,
	MyTextEdit* self
	)
{
}

MyTextEdit*
MyTextEdit::operatorNew (jnc::ClassType* type)
{
	jnc::ApiObjBox <MyTextEdit>* textEdit = (jnc::ApiObjBox <MyTextEdit>*) jnc::StdLib::gcAllocate (type);
	textEdit->prime (type);	
	textEdit->construct ();
	return textEdit;
}

void
MyTextEdit::construct ()
{
	m_qtLineEdit = new QLineEdit;
	MyWidget::construct (m_qtLineEdit);
}

//.............................................................................
