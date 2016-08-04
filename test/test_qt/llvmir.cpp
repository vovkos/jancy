#include "pch.h"
#include "llvmir.h"
#include "llvmirhighlighter.h"
#include "moc_llvmir.cpp"

LlvmIr::LlvmIr(QWidget *parent)
	: LlvmIrBase(parent)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	setLineWrapMode (QPlainTextEdit::NoWrap);
	setupHighlighter();
}

bool LlvmIr::build(jnc::Module* module)
{
	clear ();

	appendText (module->createLlvmIrString_v ());

	return true;
}


void LlvmIr::setupHighlighter()
{
	highlighter = new LlvmIrHighlighter(document());
}