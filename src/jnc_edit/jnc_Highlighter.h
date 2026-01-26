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

#include "jnc_HighlighterBase.h"

namespace jnc {

//..............................................................................

class JancyHighlighter: public lex::QtRagelSyntaxHighlighter<JancyHighlighter, HighlighterBase> {
protected:
	typedef lex::QtRagelSyntaxHighlighter<JancyHighlighter, HighlighterBase> BaseClass;

	enum BlockState {
		BlockState_Normal,
		BlockState_CommentMl,
		BlockState_LitMl,
		BlockState_LitMlRaw,
	};

public:
	const jnc::EditTheme* m_theme; // freely adjustible

public:
	JancyHighlighter(
		QTextDocument* document,
		const EditTheme* theme
	):
		BaseClass(document) {
		m_theme = theme;
	}

protected:
	void
	highlightLastToken(EditTheme::Role role) {
		BaseClass::highlightLastToken(m_theme->color(role));
	}

public:
	void
	init();

	void
	exec();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

QString
highlightJancySource(
	const QString& source,
	const EditTheme* theme
);

//..............................................................................

} // namespace jnc
