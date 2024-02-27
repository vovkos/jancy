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
	m_fmtLiteralToken = NULL;
	m_mlLiteralToken = NULL;
	m_mlBinLiteralTokenRadix = 0;
	m_bodyToken = NULL;
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

	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));

	const char* p = ts + prefix;
	size_t length = token->m_pos.m_length - prefix;
	if (length) {
		char last = te[-1];
		if (last == '\'' || last == '\\')
			length--;
	}

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
	ASSERT(token->m_pos.m_length >= 1);

	const char* p = ts + prefix;
	size_t length = token->m_pos.m_length - prefix;
	if (length) {
		char last = te[-1];
		if (last == '\"' || last == '\\')
			length--;
	}

	if (useEscapeEncoding)
		token->m_data.m_string = enc::EscapeEncoding::decode(sl::StringRef(p, length));
	else
		token->m_data.m_string = sl::StringRef(p, length);

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

// multi-line literals

Token*
Lexer::preCreateMlLiteralToken(int radix) {
	ASSERT(!m_mlLiteralToken);
	m_mlLiteralToken = preCreateToken(0);
	m_mlBinLiteralTokenRadix = radix;
	return m_mlLiteralToken;
}

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
normalizeMlLiteral(
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

Token*
Lexer::createMlLiteralToken() {
	ASSERT(m_mlLiteralToken);
	Token* token = m_mlLiteralToken;
	int radix = m_mlBinLiteralTokenRadix;

	m_mlLiteralToken = NULL;
	m_mlBinLiteralTokenRadix = 0;

	size_t left = token->m_pos.m_length;
	size_t right = te - ts;

	token->m_pos.m_length = te - token->m_pos.m_p;
	ASSERT(token->m_pos.m_length >= left + right);

	const char* p = token->m_pos.m_p + left;
	size_t length = token->m_pos.m_length - (left + right);

	if (radix) {
		token->m_token = TokenKind_BinLiteral;
		decodeByteString(&token->m_data.m_binData, radix, sl::StringRef(p, length));
	} else {
		token->m_token = TokenKind_Literal;
		token->m_data.m_string = sl::StringRef(p, length);

		if (right > 4 && ts[0] == '\n') { // \n (indent) """ -- hence, 4
			sl::String normalizedString;
			bool hasCommonIndent = normalizeMlLiteral(&normalizedString, sl::StringRef(p, length), sl::StringRef(ts + 1, right - 4));
			if (hasCommonIndent)
				token->m_data.m_string = normalizedString;
		}
	}

	return token;
}

// formatting literals

Token*
Lexer::preCreateFmtLiteralToken() {
	ASSERT(!m_fmtLiteralToken);
	m_fmtLiteralToken = preCreateToken(0);
	return m_fmtLiteralToken;
}

Token*
Lexer::createFmtLiteralToken(
	TokenKind tokenKind,
	int param
) {
	ASSERT(m_fmtLiteralToken);
	Token* token = m_fmtLiteralToken;
	m_fmtLiteralToken = NULL;

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
	return token;
}

void
Lexer::createFmtSimpleIdentifierTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, false);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_Identifier, 1, 0);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
}

void
Lexer::createFmtReGroupTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, false);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_ReGroup, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
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

	preCreateFmtLiteralToken();
}

void
Lexer::createFmtIndexTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, true);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_FmtIndex, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
}

void
Lexer::createFmtSimpleSpecifierTokens() {
	createFmtLiteralToken(TokenKind_FmtLiteral, true);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_FmtSpecifier);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
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

bool
Lexer::onLeftCurlyBrace() {
	if (!(m_flags & LexerFlag_Parse)) {
		createToken('{');
		return false;
	}

	ASSERT(m_curlyBraceLevel == 0);
	m_bodyToken = preCreateToken(TokenKind_Body);
	m_curlyBraceLevel = 1;
	return true;
}

bool
Lexer::onRightCurlyBrace() {
	ASSERT((m_flags & LexerFlag_Parse) && m_bodyToken == m_tokenList.getTail().p() && m_curlyBraceLevel);

	if (--m_curlyBraceLevel)
		return false;

	m_bodyToken->m_pos.m_length = te - m_bodyToken->m_pos.m_p;
	m_bodyToken->m_data.m_string = sl::StringRef(m_bodyToken->m_pos.m_p, m_bodyToken->m_pos.m_length);
	return true;
}

void
Lexer::onLeftParentheses() {
	if (!m_parenthesesLevelStack.isEmpty())
		m_parenthesesLevelStack[m_parenthesesLevelStack.getCount() - 1]++;

	createToken('(');
}

bool
Lexer::onRightParentheses() {
	if (!m_parenthesesLevelStack.isEmpty()) {
		size_t i = m_parenthesesLevelStack.getCount() - 1;
		if (m_parenthesesLevelStack[i] == 1) {
			m_parenthesesLevelStack.pop();
			preCreateFmtLiteralToken();
			return false;
		}

		m_parenthesesLevelStack[i]--;
	}

	createToken(')');
	return true;
}

bool
Lexer::onSemicolon() {
	ASSERT(*ts == ';');

	if (!m_parenthesesLevelStack.isEmpty()) {
		size_t i = m_parenthesesLevelStack.getCount() - 1;
		if (m_parenthesesLevelStack[i] == 1) {
			p = ts - 1; // need to reparse semicolon with 'fmt_spec' machine
			return false;
		}
	}

	createToken(';');
	return true;
}

void
Lexer::terminateFmtSpecifier() {
	ASSERT(!m_parenthesesLevelStack.isEmpty() && m_fmtLiteralToken == NULL);
	m_parenthesesLevelStack.pop();
	preCreateFmtLiteralToken();
}

//..............................................................................

} // namespace ct
} // namespace jnc
