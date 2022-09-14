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

#include "jnc_EditTheme.h"

namespace jnc {

//..............................................................................

class JancyHighlighter: public lex::QtRagelSyntaxHighlighter<JancyHighlighter> {
protected:
	enum BlockState {
		BlockState_Normal,
		BlockState_CommentSl,
		BlockState_CommentMl,
		BlockState_LitMl,
	};

public:
	const jnc::EditTheme* m_theme; // freely adjustible

public:
	JancyHighlighter(
		QTextDocument* document,
		const EditTheme* theme
	):
		lex::QtRagelSyntaxHighlighter<JancyHighlighter>(document) {
		m_theme = theme;
	}

protected:
	bool
	isTokenSuffix(const sl::StringRef& suffix) {
		return sl::StringRef(ts, te - ts).isSuffix(suffix);
	}

	bool
	isTokenSuffix(
		const char* p,
		size_t length
	) {
		return sl::StringRef(ts, te - ts).isSuffix(sl::StringRef(p, length));
	}

	void
	highlightLastToken(EditTheme::Role role) {
		lex::QtRagelSyntaxHighlighter<JancyHighlighter>::highlightLastToken(m_theme->color(role));
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
