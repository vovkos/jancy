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

//..............................................................................

class LlvmIrHighlighter: public gui::QtRagelSyntaxHighlighter<LlvmIrHighlighter> {
public:
	enum Color {
		Color_Keyword  = 0x0000ff,
		Color_Constant = 0xce7b00,
		Color_Comment  = 0x969696,
	};

public:
	LlvmIrHighlighter(QTextDocument *parent):
		gui::QtRagelSyntaxHighlighter<LlvmIrHighlighter>(parent) {}

	void
	init();

	void
	exec();
};

//..............................................................................
