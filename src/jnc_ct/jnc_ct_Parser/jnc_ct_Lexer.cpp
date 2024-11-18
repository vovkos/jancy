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
#include "jnc_ct_Lexer.h"
#include "jnc_ct_Lexer.rl.cpp"

namespace jnc {
namespace ct {

//..............................................................................

void
cloneTokenList(
    sl::List<Token>* clone,
    const sl::List<Token>& list
) {
    clone->clear();
    mem::Pool<Token>* tokenPool = mem::getCurrentThreadPool<Token>();
    sl::ConstIterator<Token> it = list.getHead();
    for (; it; it++)
        clone->insertTail(tokenPool->get(**it));
}

//..............................................................................

inline
bool
isByteStringSep(uchar_t c) {
	switch (c) {
	case ' ': case '\t': case '\r': case '\n':
	case '.': case ',': case ';': case ':':
		return true;

	default:
		return false;
	}
}

size_t
decodeByteString(
	sl::Array<char>* buffer,
	int radix, // must be 2, 8, 10 or 16
	const sl::StringRef& string
) {
	ASSERT(radix == 2 || radix == 8 || radix == 10 || radix == 16);

	enum State {
		State_Sep = 0,
		State_Byte,
	};

	State state = State_Sep;

	buffer->clear();
	buffer->reserve(string.getLength() / 2); // good estimate no matter the radix

	char byteBuffer[16] = { 0 }; // big enough to fit byte in any radix
	char* byteEnd;
	size_t byteLength;

	size_t maxByteLength = radix == 16 ? 2 : radix == 2 ? 8 : 3;

	uchar_t x;

	const char* p = string.cp();
	const char* end = string.getEnd();
	for (; p < end; p++) {
		uchar_t c = *p;
		bool_t isSep = isByteStringSep(c);

		switch (state) {
		case State_Sep:
			if (isSep)
				break;

			byteBuffer[0] = c;
			byteLength = 1;
			state = State_Byte;
			break;

		case State_Byte:
			if (!isSep) {
				byteBuffer[byteLength++] = c;
				if (byteLength < maxByteLength)
					break;
			}

			byteBuffer[byteLength] = 0;
			x = (uchar_t)strtoul(byteBuffer, &byteEnd, radix);

			if (byteEnd == &byteBuffer[byteLength])
				buffer->append(x);
			else
				p = end; // not a proper byte string anymore, break the loop

			state = State_Sep;
			break;
		}
	}

	if (state == State_Byte) {
		byteBuffer[byteLength] = 0;
		x = (uchar_t)strtoul(byteBuffer, &byteEnd, radix);

		if (byteEnd == &byteBuffer[byteLength])
			buffer->append(x);
	}

	return buffer->getCount();
}

//..............................................................................

Lexer::Lexer(uint_t flags) {
	m_flags = flags;
	m_bodyToken = NULL;
	m_literalExToken = NULL;
	m_literalExInfo.m_literalKind = LiteralExKind_Undefined;
	m_literalExInfo.m_indent = NULL;
	m_literalExInfo.m_indentLength = 0;
	m_curlyBraceLevel = 0;
}

size_t
Lexer::tokenizeString(
	sl::List<Token>* tokenList,
	const sl::StringRef& source,
	lex::RagelBomMode bomMode
) {
	Lexer lexer;
	lexer.create(sl::StringRef(), source, bomMode);
	while (lexer.getToken()->m_tokenKind > 0)
		tokenList->insertTail(lexer.takeToken());

	return tokenList->getCount();
}

Token*
Lexer::createStringToken(
	TokenKind tokenKind,
	size_t prefix,
	size_t suffix
) {
	Token* token = createToken(tokenKind);
	ASSERT(token->m_pos.m_length >= prefix + suffix);

	size_t length = token->m_pos.m_length - prefix - suffix;
	token->m_data.m_string = sl::StringRef(ts + prefix, length);
	return token;
}

Token*
Lexer::createCharToken(
	size_t prefix,
	bool useEscapeEncoding
) {
	Token* token = createToken(TokenKind_Integer);
	ASSERT(token->m_pos.m_length >= 1);

	const char* p = ts + prefix;
	size_t length = token->m_pos.m_length - prefix;
	if (length) {
		char last = te[-1];
		if (last == '\'' || last == '\\')
			length--;
	}

	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));

	if (useEscapeEncoding) {
		enc::EscapeEncoding::decode(&string, sl::StringRef(p, length));
		p = string;
		length = string.getLength();
	}

	if (length > sizeof(int))
		length = sizeof(int);

	int result = 0;
	int shift = 8 * (length - 1);

	const char* end = p + length;
	for (; p < end; p++, shift -= 8)
		result |= *(uchar_t*)p << shift;

	token->m_data.m_integer = result;
	return token;
}

Token*
Lexer::createLiteralToken(
	size_t prefix,
	bool useEscapeEncoding
) {
	Token* token = createToken(TokenKind_Literal);
	ASSERT(token->m_pos.m_length >= prefix);

	const char* p = ts + prefix;
	size_t length = token->m_pos.m_length - prefix;
	if (length) {
		char last = te[-1];
		if (last == '\"' || last == '\\')
			length--;
	}

	token->m_data.m_string = !useEscapeEncoding ?
		sl::StringRef(p, length) :
		enc::EscapeEncoding::decode(sl::StringRef(p, length));

	return token;
}

Token*
Lexer::createSourceFileToken() {
	Token* token = createToken(TokenKind_Literal);
	token->m_data.m_string = m_filePath;
	return token;
}

Token*
Lexer::createSourceDirToken() {
	if (m_dir.isEmpty())
		m_dir = m_filePath.isEmpty() ?
			io::getCurrentDir() :
			io::getDir(m_filePath);

	Token* token = createToken(TokenKind_Literal);
	token->m_data.m_string = m_dir;
	return token;
}

Token*
Lexer::createBinLiteralToken(int radix) {
	Token* token = createToken(TokenKind_BinLiteral);
	ASSERT(token->m_pos.m_length >= 4);
	decodeByteString(&token->m_data.m_binData, radix, sl::StringRef(ts + 3, token->m_pos.m_length - 4));
	return token;
}

Token*
Lexer::createIntegerToken(
	TokenKind tokenKind,
	int radix,
	size_t left
) {
	Token* token = createToken(tokenKind);
	token->m_data.m_int64_u = _strtoui64(ts + left, NULL, radix);
	return token;
}

Token*
Lexer::createFpToken() {
	Token* token = createToken(TokenKind_Fp);
	token->m_data.m_double = strtod(ts, NULL);
	return token;
}

Token*
Lexer::createConstIntegerToken(int value) {
	Token* token = createToken(TokenKind_Integer);
	token->m_data.m_integer = value;
	return token;
}

Token*
Lexer::preCreateLiteralEx(LiteralExKind literalKind) {
	ASSERT(!m_literalExToken && !m_literalExInfo.m_literalKind);
	m_literalExToken = preCreateToken(0);
	m_literalExInfo.m_literalKind = literalKind;
	return m_literalExToken;
}

Token*
Lexer::preCreateMlLiteral(
	LiteralExKind literalKind,
	size_t minTokenLength
) {
	ASSERT(!m_literalExToken && !m_literalExInfo.m_literalKind);
	Token* token = preCreateLiteralEx(literalKind);
	if (token->m_pos.m_length > minTokenLength) {
		m_literalExInfo.m_indentLength = te - m_literalExInfo.m_indent;
		token->m_pos.m_length -= m_literalExInfo.m_indentLength; // indent goes to the literal
	} else
		m_literalExInfo.m_indentLength = 0;

	return token;
}

// multi-line literals

sl::String
Lexer::unindentMlLiteral(
	const sl::StringRef& source,
	size_t indentLength
) {
	ASSERT(indentLength && indentLength < source.getLength());

	sl::String string;

	const char* p = source.cp() + indentLength;
	const char* end = source.getEnd();
	while (p < end) {
		size_t length = end - p;
		const char* nl = (char*)memchr(p, '\n', length);
		if (!nl) {
			string.append(p, length);
			break;
		}

		nl++;
		string.append(p, nl - p);
		p = nl + indentLength; // skip common indent
	}

	return string;
}

void
Lexer::updateMlLiteralIndent() {
	ASSERT(*ts == '\n' && m_literalExToken && m_literalExInfo.m_literalKind >= LiteralExKind_Ml);

	if (!m_literalExInfo.m_indentLength) // shortcut
		return;

	const char* indent = ts + 1;
	size_t indentLength = te - indent;
	if (m_literalExInfo.m_indentLength > indentLength)
		m_literalExInfo.m_indentLength = indentLength;

	for (size_t i = 0; i < m_literalExInfo.m_indentLength; i++) {
		if (m_literalExInfo.m_indent[i] != indent[i]) {
			m_literalExInfo.m_indentLength = i;
			break;
		}
	}
}

void
Lexer::finalizeMlLiteralToken() {
	ASSERT(m_literalExToken && m_literalExInfo.m_literalKind >= LiteralExKind_Ml);

	Token* token = m_literalExToken; // short alias
	size_t left = token->m_pos.m_length;
	size_t right = te - ts;

	token->m_pos.m_length = te - token->m_pos.m_p;
	ASSERT(token->m_pos.m_length >= left + right);

	const char* p = token->m_pos.m_p + left;
	size_t length = token->m_pos.m_length - (left + right);
	sl::StringRef string(p, length);

	if (m_literalExInfo.m_literalKind > LiteralExKind__RadixBase) {
		int radix = m_literalExInfo.m_literalKind - LiteralExKind__RadixBase;
		token->m_token = TokenKind_BinLiteral;
		decodeByteString(&token->m_data.m_binData, radix, string);
	} else {
		token->m_token = TokenKind_Literal;

		if (m_literalExInfo.m_indentLength) // first, remove common indent
			string = unindentMlLiteral(string, m_literalExInfo.m_indentLength);

		if (m_literalExInfo.m_literalKind == LiteralExKind_MlEsc) // then esc-decode
			string = enc::EscapeEncoding::decode(string);

		token->m_data.m_string = string;
	}

#if (_AXL_DEBUG)
	m_literalExToken = NULL;
	m_literalExInfo.m_literalKind = LiteralExKind_Undefined;
#endif
}

// formatting literals

void
Lexer::createFmtLiteralToken(
	TokenKind tokenKind,
	uint_t flags
) {
	ASSERT(m_literalExToken && (m_literalExInfo.m_literalKind == LiteralExKind_Fmt || m_literalExInfo.m_literalKind == LiteralExKind_FmtMl));

	Token* token = m_literalExToken;
	size_t left = token->m_pos.m_length;
	size_t right = te - ts;

	token->m_pos.m_length = te - token->m_pos.m_p;
	ASSERT(token->m_pos.m_length >= left + right);

	token->m_token = tokenKind;
	token->m_data.m_string = enc::EscapeEncoding::decode(sl::StringRef(
		token->m_pos.m_p + left,
		token->m_pos.m_length - (left + right)
	));

	token->m_data.m_integer = flags;

#if (_AXL_DEBUG)
	m_literalExToken = NULL;
#endif
}

void
Lexer::createFmtOpenerToken(uint_t flags) {
	ASSERT(m_literalExToken && (m_literalExInfo.m_literalKind == LiteralExKind_Fmt || m_literalExInfo.m_literalKind == LiteralExKind_FmtMl));

	if (*ts == '%')
		flags |= FmtLiteralTokenFlag_Index;

	createFmtLiteralToken(TokenKind_FmtLiteral, flags);

	FmtLiteralStackEntry entry;
	(LiteralExInfo&)entry = m_literalExInfo;
	entry.m_level = 1;
	entry.m_leftBraceChar = ts[1];
	m_fmtLiteralStack.append(entry);

#if (_AXL_DEBUG)
	m_literalExInfo.m_literalKind = LiteralExKind_Undefined;
#endif
}

void
Lexer::createFmtSimpleIdentifierTokens(uint_t flags) {
	createFmtLiteralToken(TokenKind_FmtLiteral, flags);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_Identifier, 1, 0);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtReGroupTokens(uint_t flags) {
	createFmtLiteralToken(TokenKind_FmtLiteral, flags);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_ReGroup, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtLastErrorDescriptionTokens(uint_t flags) {
	createFmtLiteralToken(TokenKind_FmtLiteral, flags);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	Token* token = createToken(TokenKind_Identifier);
	token->m_data.m_string = "std";
	createToken('.');
	token = createToken(TokenKind_Identifier);
	token->m_data.m_string = "getLastError";
	createToken('(');
	createToken(')');
	createToken('.');
	token = createToken(TokenKind_Identifier);
	token->m_data.m_string = "m_description";

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtIndexTokens(uint_t flags) {
	createFmtLiteralToken(TokenKind_FmtLiteral, flags | FmtLiteralTokenFlag_Index);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_FmtIndex, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtSimpleSpecifierTokens(uint_t flags) {
	createFmtLiteralToken(TokenKind_FmtLiteral, flags | FmtLiteralTokenFlag_Index);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_FmtSpecifier);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

Token*
Lexer::createFmtSpecifierToken() {
	ASSERT(*ts == ';');
	ts++;

	while (ts < te && (*ts == ' ' || *ts == '\t'))
		ts++;

	return ts < te ? createStringToken(TokenKind_FmtSpecifier) : NULL;
}

void
Lexer::createDynamicCastTokens() {
	Token* token = createToken(TokenKind_DynamicCast);
	token->m_pos.m_length = lengthof("dynamic");

	token = createToken('(');
	size_t delta = token->m_pos.m_length - 1;
	token->m_pos.m_p += delta;
	token->m_pos.m_col += delta;
	token->m_pos.m_length = 1;
}

Token*
Lexer::createDoxyCommentToken(TokenKind tokenKind) {
	if (!(m_flags & LexerFlag_DoxyComments))
		return NULL;

	ASSERT(te - ts >= 3 && *ts == '/');
	size_t suffix = 0;

	if (tokenKind >= TokenKind_DoxyComment3) { // multiline c-style: /** or /*!
		ASSERT(ts[1] == '*' && te[-1] == '/' && te[-2] == '*');
		suffix = 2;
	}

	return createStringToken(tokenKind, 3, suffix);
}

void
Lexer::onLeftBrace(char leftBraceChar) {
	createToken(leftBraceChar);

	if (!m_fmtLiteralStack.isEmpty()) {
		FmtLiteralStackEntry& entry = m_fmtLiteralStack.getBack();
		if (entry.m_leftBraceChar == leftBraceChar)
			entry.m_level++;
	}
}

bool
Lexer::onRightBrace(
	char leftBraceChar,
	char rightBraceChar
) {
	if (!m_fmtLiteralStack.isEmpty()) {
		FmtLiteralStackEntry& entry = m_fmtLiteralStack.getBack();
		if (entry.m_leftBraceChar == leftBraceChar && !--entry.m_level) {
			m_literalExToken = preCreateToken(0);
			m_literalExInfo = entry;
			m_fmtLiteralStack.pop();
			return true;
		}
	}

	createToken(rightBraceChar);
	return false;
}

bool
Lexer::onLeftCurlyBrace() {
	ASSERT(*ts == '{');

	if (m_flags & LexerFlag_Parse) { // in parse mode, we create TokenKind_Body
		ASSERT(m_curlyBraceLevel == 0);
		m_bodyToken = preCreateToken(TokenKind_Body);
		m_curlyBraceLevel = 1;
		return true;
	} else {
		onLeftBrace('{');
		return false;
	}
}

bool
Lexer::onRightCurlyBrace() {
	ASSERT(*ts == '}');

	if (!(m_flags & LexerFlag_Parse))
		return onRightBrace('{', '}');

	ASSERT(m_bodyToken == m_tokenList.getTail().p() && m_curlyBraceLevel);

	if (--m_curlyBraceLevel)
		return false;

	m_bodyToken->m_pos.m_length = te - m_bodyToken->m_pos.m_p;
	m_bodyToken->m_data.m_string = sl::StringRef(m_bodyToken->m_pos.m_p, m_bodyToken->m_pos.m_length);
	return true;
}

bool
Lexer::onSemicolon() {
	ASSERT(*ts == ';');

	if (!m_fmtLiteralStack.isEmpty()) {
		if (m_fmtLiteralStack.getBack().m_level == 1) {
			p = ts - 1; // need to reparse semicolon with 'fmt_spec' machine
			return true;
		}
	}

	createToken(';');
	return false;
}

void
Lexer::terminateFmtSpecifier() {
	ASSERT(!m_literalExToken && !m_literalExInfo.m_literalKind && !m_fmtLiteralStack.isEmpty());

	m_literalExToken = preCreateToken(0);
	m_literalExInfo = m_fmtLiteralStack.getBack();
	m_fmtLiteralStack.pop();
}

//..............................................................................

} // namespace ct
} // namespace jnc
