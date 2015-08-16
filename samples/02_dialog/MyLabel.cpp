#include "pch.h"
#include "MyLabel.h"

//.............................................................................

void
MyLabel::construct (jnc::DataPtr textPtr)
{
	m_qtLabel = new QLabel;
	MyWidget::construct (m_qtLabel);
	m_color = -1;
	m_backColor = -1;
	m_alignment = m_qtLabel->alignment ();
	setText (textPtr);
}

MyLabel*
MyLabel::operatorNew (
	jnc::ClassType* type,
	jnc::DataPtr textPtr
	)
{
	MyLabel* label = (MyLabel*) jnc::StdLib::allocateClass (type);
	label->construct (textPtr);
	return label;
}

void
MyLabel::updateStyleSheet ()
{
	QString styleSheet = "QLabel { ";

	if (m_color != -1)
		styleSheet += QString ("color: #%1; ").arg (m_color, 6, 16, (QChar) '0');
	else
		styleSheet += "color: black; ";

	if (m_backColor != -1)
		styleSheet += QString ("background-color: #%1; ").arg (m_backColor, 6, 16, (QChar) '0');
	else
		styleSheet += "background-color: transparent; ";

	styleSheet += "}";

	m_qtLabel->setStyleSheet (styleSheet);
}

//.............................................................................
