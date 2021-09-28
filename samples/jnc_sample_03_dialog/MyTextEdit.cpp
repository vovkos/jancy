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
#include "MyTextEdit.h"
#include "MyLib.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	MyTextEdit,
	"TextEdit",
	g_myLibGuid,
	MyLibCacheSlot_TextEdit,
	MyTextEdit,
	NULL)

JNC_BEGIN_TYPE_FUNCTION_MAP(MyTextEdit)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<MyTextEdit>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<MyTextEdit>)
	JNC_MAP_PROPERTY("m_text", &MyTextEdit::getText, &MyTextEdit::setText)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

MyTextEdit::MyTextEdit():
	MyWidget(new QLineEdit) {
	m_qtLineEdit = (QLineEdit*)m_handle;
}

MyTextEdit::~MyTextEdit() {
	if (!m_qtLineEdit->parent())
		delete m_qtLineEdit;

	delete m_onTextChangedBridge;
}

//..............................................................................
