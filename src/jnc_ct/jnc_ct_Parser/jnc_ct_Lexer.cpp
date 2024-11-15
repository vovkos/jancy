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
	m_literalExKind = LiteralExKind_Undefined;
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
	ASSERT(!m_literalExToken && !m_literalExKind);
	m_literalExToken = preCreateToken(0);
	m_literalExKind = literalKind;
	return m_literalExToken;
}

// multi-line literals

size_t
getIndentLength(const sl::StringRef& string) {
	const char* p = string.cp();
	const char* end = string.getEnd();
	const char* p0 = p;

	for (; p < end; p++) {
		char c = *p;
		if (c != ' ' && c != '\t' && c != '\r')
			break;
	}

	return p - p0;
}

bool
unindent(
	sl::String* string,
	const sl::StringRef& source,
	const sl::StringRef& indent
) {
	size_t indentLength = indent.getLength();
	ASSERT(source.getLength() >= indentLength);

	string->clear();

	const char* p = source.cp();
	const char* end = source.getEnd();
	while (p < end) {
		size_t chunkLength = end - p;
		ASSERT(chunkLength > indentLength);

		size_t lineIndentLength = getIndentLength(sl::StringRef(p, chunkLength));

		bool isEmpty = p[lineIndentLength] == '\n';
		if (isEmpty) {
			if (lineIndentLength && p[lineIndentLength - 1] == '\r')
				string->append("\r\n", 2);
			else
				string->append('\n');

			p += lineIndentLength + 1;
		} else {
			if (lineIndentLength < indentLength || memcmp(p, indent.cp(), indentLength) != 0)
				return false;

			p += indentLength;

			const char* nl = strchr(p, '\n');
			ASSERT(nl);

			string->append(p, nl - p + 1);
			p = nl + 1;
		}
	}

	return true;
}

void
Lexer::finalizeMlLiteralToken() {
	ASSERT(m_literalExToken && m_literalExKind >= LiteralExKind_Ml);

	Token* token = m_literalExToken; // short alias
	size_t left = token->m_pos.m_length;
	size_t right = te - ts;

	bool hasNl = *ts == '\n';
	if (hasNl)
		right--; // include '\n' into literal

	token->m_pos.m_length = te - token->m_pos.m_p;
	ASSERT(token->m_pos.m_length >= left + right);

	const char* p = token->m_pos.m_p + left;
	size_t length = token->m_pos.m_length - (left + right);

	if (m_literalExKind > LiteralExKind__RadixBase) {
		token->m_token = TokenKind_BinLiteral;
		decodeByteString(&token->m_data.m_binData, m_literalExKind - LiteralExKind__RadixBase, sl::StringRef(p, length));
	} else {
		token->m_token = TokenKind_Literal;

		sl::StringRef string(p, length);
		if (m_literalExKind == LiteralExKind_MlEsc)
			string = enc::EscapeEncoding::decode(string);

		if (hasNl && right > 3) {
			sl::String normalizedString;
			sl::StringRef indent(ts + 1, right - 3);

			bool hasCommonIndent = unindent(&normalizedString, string, indent);
			if (hasCommonIndent)
				string = normalizedString;
		}

		token->m_data.m_string = string;
	}

	m_literalExToken = NULL;
	m_literalExKind = LiteralExKind_Undefined;
}

// formatting literals

void
Lexer::createFmtLiteralToken(
	TokenKind tokenKind,
	int param
) {
	ASSERT(m_literalExToken && (m_literalExKind == LiteralExKind_Fmt || m_literalExKind == LiteralExKind_FmtMl));

	Token* token = m_literalExToken;
	m_literalExToken = NULL;

	size_t left = token->m_pos.m_length;
	size_t right = te - ts;

	token->m_pos.m_length = te - token->m_pos.m_p;
	ASSERT(token->m_pos.m_length >= left + right);

	token->m_token = tokenKind;
	token->m_data.m_string = enc::EscapeEncoding::decode(sl::StringRef(
		token->m_pos.m_p + left,
		token->m_pos.m_length - (left + right)
	));

	token->m_data.m_integer = param;
}

void
Lexer::createFmtOpenerToken() {
	ASSERT(m_literalExToken && (m_literalExKind == LiteralExKind_Fmt || m_literalExKind == LiteralExKind_FmtMl));

	createFmtLiteralToken(TokenKind_FmtLiteral, *ts == '%');

	FmtLiteralStackEntry entry;
	entry.m_level = 1;
	entry.m_leftBraceChar = ts[1];
	entry.m_literalKind = m_literalExKind;
	m_fmtLiteralStack.append(entry);

	m_literalExKind = LiteralExKind_Undefined;
}

void
Lexer::createFmtSimpleIdentifierTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, false);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_Identifier, 1, 0);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtReGroupTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, false);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_ReGroup, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtLastErrorDescriptionTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, false);

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
Lexer::createFmtIndexTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, true);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_FmtIndex, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;
	m_literalExToken = preCreateToken(0);
}

void
Lexer::createFmtSimpleSpecifierTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, true);

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
			LiteralExKind literalKind = entry.m_literalKind;
			m_fmtLiteralStack.pop();
			preCreateLiteralEx(literalKind);
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
	ASSERT(!m_literalExToken && !m_literalExKind && !m_fmtLiteralStack.isEmpty());

	LiteralExKind literalKind = m_fmtLiteralStack.getBack().m_literalKind;
	m_fmtLiteralStack.pop();
	preCreateLiteralEx(literalKind);
}

//..............................................................................

} // namespace ct
} // namespace jnc
