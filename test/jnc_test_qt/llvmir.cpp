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
#include "llvmir.h"
#include "llvmirhighlighter.h"
#include "moc_llvmir.cpp"

LlvmIr::LlvmIr(QWidget *parent)
	: LlvmIrBase(parent)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	setLineWrapMode(QPlainTextEdit::NoWrap);
	setupHighlighter();
}

bool LlvmIr::build(jnc::Module* module)
{
	clear();

	appendText(module->getLlvmIrString_v());

	return true;
}

void LlvmIr::setupHighlighter()
{
	highlighter = new LlvmIrHighlighter(document());
}
