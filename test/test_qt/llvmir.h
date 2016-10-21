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

#ifndef _LLVMIR_H
#define _LLVMIR_H

#include "editor.h"

class LlvmIrHighlighter;

#define LlvmIrBase Editor

class LlvmIr : public LlvmIrBase
{
	Q_OBJECT

public:
	LlvmIr(QWidget *parent);

	QSize sizeHint() const { return QSize(300, 50); }
	bool build (jnc::Module* module);

private:
	void setupHighlighter();

	LlvmIrHighlighter *highlighter;
};

#endif
