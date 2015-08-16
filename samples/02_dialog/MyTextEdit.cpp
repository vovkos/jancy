#include "pch.h"
#include "MyTextEdit.h"

//.............................................................................

void
MyTextEdit::construct ()
{
	m_qtLineEdit = new QLineEdit;
	MyWidget::construct (m_qtLineEdit);
}

MyTextEdit*
MyTextEdit::operatorNew (jnc::ClassType* type)
{
	MyTextEdit* textEdit = (MyTextEdit*) jnc::StdLib::allocateClass (type);
	textEdit->construct ();
	return textEdit;
}

//.............................................................................
