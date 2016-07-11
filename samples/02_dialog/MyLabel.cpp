#include "pch.h"
#include "MyLabel.h"

//.............................................................................

MyLabel::MyLabel (jnc::DataPtr textPtr):
	MyWidget (new QLabel)
{
	m_qtLabel = (QLabel*) m_handle;
	m_color = -1;
	m_backColor = -1;
	m_alignment = m_qtLabel->alignment ();
	setText (textPtr);
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
