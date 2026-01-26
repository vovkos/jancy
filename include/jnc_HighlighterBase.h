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

class HighlighterBase: public QSyntaxHighlighter {
public:
	HighlighterBase(QTextDocument* document): QSyntaxHighlighter(document) {
		m_theme = NULL;
	}

	const jnc::EditTheme* theme() {
		return m_theme;
	}

	void setTheme(const jnc::EditTheme* theme) {
		m_theme = theme;
		rehighlight();
	}

protected:
	const jnc::EditTheme* m_theme;
};

//..............................................................................

} // namespace jnc
