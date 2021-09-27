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

#pragma once

#include "monospaceplaintextedit.h"

class LlvmIrHighlighter;

//..............................................................................

class LlvmIr: public MonospacePlainTextEdit {
	Q_OBJECT

public:
	LlvmIr(QWidget* parent);

	QSize sizeHint() const { return QSize(300, 50); }
	bool build(jnc::Module* module);

protected:
	LlvmIrHighlighter* m_highlighter;
};

//..............................................................................
