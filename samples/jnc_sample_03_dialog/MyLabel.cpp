//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "MyLabel.h"
#include "MyLib.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	MyLabel,
	"Label",
	g_myLibGuid,
	MyLibCacheSlot_Label,
	MyLabel,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(MyLabel)
	JNC_MAP_CONSTRUCTOR(&(jnc::construct<MyLabel, jnc::DataPtr>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<MyLabel>)
	JNC_MAP_AUTOGET_PROPERTY("m_text", &MyLabel::setText)
	JNC_MAP_AUTOGET_PROPERTY("m_color", &MyLabel::setColor)
	JNC_MAP_AUTOGET_PROPERTY("m_backColor", &MyLabel::setBackColor)
	JNC_MAP_AUTOGET_PROPERTY("m_alignment", &MyLabel::setAlignment)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

MyLabel::MyLabel(jnc::DataPtr textPtr):
	MyWidget(new QLabel)
{
	m_qtLabel = (QLabel*)m_handle;
	m_color = -1;
	m_backColor = -1;
	m_alignment = m_qtLabel->alignment();
	setText(textPtr);
}

MyLabel::~MyLabel()
{
	if (!m_qtLabel->parent())
		delete m_qtLabel;
}

void
MyLabel::updateStyleSheet()
{
	QString styleSheet = "QLabel { ";

	if (m_color != -1)
		styleSheet += QString("color: #%1; ").arg (m_color, 6, 16, (QChar) '0');
	else
		styleSheet += "color: black; ";

	if (m_backColor != -1)
		styleSheet += QString("background-color: #%1; ").arg (m_backColor, 6, 16, (QChar) '0');
	else
		styleSheet += "background-color: transparent; ";

	styleSheet += "}";

	m_qtLabel->setStyleSheet(styleSheet);
}

//..............................................................................
