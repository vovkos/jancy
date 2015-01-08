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
MyTextEdit::operatorNew ()
{
	jnc::ApiObjBox <MyTextEdit>* textEdit = (jnc::ApiObjBox <MyTextEdit>*) jnc::StdLib::gcAllocate (getApiType ());
	textEdit->prime ();	
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
