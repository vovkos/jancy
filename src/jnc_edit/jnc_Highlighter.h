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

namespace jnc {

//..............................................................................

class JancyHighlighter: public gui::QtRagelSyntaxHighlighter<JancyHighlighter> {
public:
	enum Color {
		Color_Keyword  = 0x0000ff,
		Color_Constant = 0xce7b00,
		Color_Comment  = 0x969696,
	};

protected:
	enum BlockState {
		BlockState_Normal,
		BlockState_CommentSl,
		BlockState_CommentMl,
		BlockState_LitMl,
	};

public:
	JancyHighlighter(QTextDocument* parent = NULL):
		gui::QtRagelSyntaxHighlighter<JancyHighlighter>(parent) {}

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

public:
	void
	init();

	void
	exec();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

QString
highlightJancySource(const QString& source);

//..............................................................................

} // namespace jnc
