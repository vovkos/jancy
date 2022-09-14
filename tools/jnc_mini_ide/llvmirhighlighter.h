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

class LlvmIrHighlighter: public lex::QtRagelSyntaxHighlighter<LlvmIrHighlighter> {
protected:
	const jnc::EditTheme* m_theme;

public:
	LlvmIrHighlighter(
		QTextDocument* document,
		const jnc::EditTheme* theme
	):
		lex::QtRagelSyntaxHighlighter<LlvmIrHighlighter>(document) {
		m_theme = theme;
	}

protected:
	void
	highlightLastToken(jnc::EditTheme::Role role) {
		lex::QtRagelSyntaxHighlighter<LlvmIrHighlighter>::highlightLastToken(m_theme->color(role));
	}

public:
	void
	init();

	void
	exec();
};

//..............................................................................
