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

inline
bool
isByteStringSep(uchar_t c)
{
	switch (c)
	{
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
	)
{
	ASSERT(radix == 2 || radix == 8 || radix == 10 || radix == 16);

	enum State
	{
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
	for (; p < end; p++)
	{
		uchar_t c = *p;
		bool_t isSep = isByteStringSep(c);

		switch (state)
		{
		case State_Sep:
			if (isSep)
				break;

			byteBuffer[0] = c;
			byteLength = 1;
			state = State_Byte;
			break;

		case State_Byte:
			if (!isSep)
			{
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

	if (state == State_Byte)
	{
		byteBuffer[byteLength] = 0;
		x = (uchar_t)strtoul(byteBuffer, &byteEnd, radix);

		if (byteEnd == &byteBuffer[byteLength])
			buffer->append(x);
	}

	return buffer->getCount();
}

//..............................................................................

Lexer::Lexer(LexerMode mode)
{
	m_mode = mode;
	m_fmtLiteralToken = NULL;
	m_mlLiteralToken = NULL;
	m_mlBinLiteralTokenRadix = 0;
	m_bodyToken = NULL;
	m_curlyBraceLevel = 0;
}

Token*
Lexer::createStringToken(
	TokenKind tokenKind,
	size_t prefix,
	size_t suffix
	)
{
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
	)
{
	Token* token = createToken(TokenKind_Integer);
	ASSERT(token->m_pos.m_length >= 1);

	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));

	const char* p = ts + prefix;
	size_t length = token->m_pos.m_length - prefix;
	if (length)
	{
		char last = te[-1];
		if (last == '\'' || last == '\\')
			length--;
	}

	if (useEscapeEncoding)
	{
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
	)
{
	Token* token = createToken(TokenKind_Literal);
	ASSERT(token->m_pos.m_length >= 1);

	const char* p = ts + prefix;
	size_t length = token->m_pos.m_length - prefix;
	if (length)
	{
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
Lexer::createSourceFileToken()
{
	Token* token = createToken(TokenKind_Literal);
	token->m_data.m_string = m_filePath;
	return token;
}

Token*
Lexer::createSourceDirToken()
{
	if (m_dir.isEmpty())
		m_dir = m_filePath.isEmpty() ?
			io::getCurrentDir() :
			io::getDir(m_filePath);

	Token* token = createToken(TokenKind_Literal);
	token->m_data.m_string = m_dir;
	return token;
}

Token*
Lexer::createBinLiteralToken(int radix)
{
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
	)
{
	Token* token = createToken(tokenKind);
	token->m_data.m_int64_u = _strtoui64(ts + left, NULL, radix);
	return token;
}

Token*
Lexer::createFpToken()
{
	Token* token = createToken(TokenKind_Fp);
	token->m_data.m_double = strtod(ts, NULL);
	return token;
}

Token*
Lexer::createConstIntegerToken(int value)
{
	Token* token = createToken(TokenKind_Integer);
	token->m_data.m_integer = value;
	return token;
}

// multi-line literals

Token*
Lexer::preCreateMlLiteralToken(int radix)
{
	ASSERT(!m_mlLiteralToken);
	m_mlLiteralToken = preCreateToken(0);
	m_mlBinLiteralTokenRadix = radix;
	return m_mlLiteralToken;
}

size_t
getWsPrefixLength(const sl::StringRef& string)
{
	const char* p = string.cp();
	const char* end = string.getEnd();
	const char* p0 = p;

	for (; p < end; p++)
	{
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
	const sl::StringRef& prefix
	)
{
	size_t prefixLength = prefix.getLength();
	ASSERT(source.getLength() >= prefixLength);

	string->clear();

	const char* p = source.cp();
	const char* end = source.getEnd();
	while (p < end)
	{
		size_t chunkLength = end - p;
		ASSERT(chunkLength > prefixLength);

		size_t linePrefixLength = getWsPrefixLength(sl::StringRef(p, chunkLength));

		bool isEmpty = p[linePrefixLength] == '\n';
		if (isEmpty)
		{
			if (linePrefixLength && p[linePrefixLength - 1] == '\r')
				string->append("\r\n", 2);
			else
				string->append('\n');

			p += linePrefixLength + 1;
		}
		else
		{
			if (linePrefixLength < prefixLength || memcmp(p, prefix.cp(), prefixLength) != 0)
				return false;

			p += prefixLength;

			const char* nl = strchr(p, '\n');
			ASSERT(nl);

			string->append(p, nl - p + 1);
			p = nl + 1;
		}
	}

	return true;
}

Token*
Lexer::createMlLiteralToken()
{
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

	if (radix)
	{
		token->m_token = TokenKind_BinLiteral;
		decodeByteString(&token->m_data.m_binData, radix, sl::StringRef(p, length));
	}
	else
	{
		token->m_token = TokenKind_Literal;
		token->m_data.m_string = sl::StringRef(p, length);

		if (right > 3 && ts[-1] == '\n')
		{
			sl::String normalizedString;
			bool hasCommonPrefix = normalizeMlLiteral(&normalizedString, sl::StringRef(p, length), sl::StringRef(ts, right - 3));
			if (hasCommonPrefix)
				token->m_data.m_string = normalizedString;
		}
	}

	return token;
}

// formatting literals

Token*
Lexer::preCreateFmtLiteralToken()
{
	ASSERT(!m_fmtLiteralToken);
	m_fmtLiteralToken = preCreateToken(0);
	return m_fmtLiteralToken;
}

Token*
Lexer::createFmtLiteralToken(
	TokenKind tokenKind,
	int param
	)
{
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
Lexer::createFmtSimpleIdentifierTokens()
{
	createFmtLiteralToken(TokenKind_FmtLiteral, false);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_Identifier, 1, 0);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
}

void
Lexer::createFmtLastErrorDescriptionTokens()
{
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
Lexer::createFmtIndexTokens()
{
	createFmtLiteralToken(TokenKind_FmtLiteral, true);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createIntegerToken(TokenKind_FmtIndex, 10, 1);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
}

void
Lexer::createFmtSimpleSpecifierTokens()
{
	createFmtLiteralToken(TokenKind_FmtLiteral, true);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	createStringToken(TokenKind_FmtSpecifier);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken();
}

Token*
Lexer::createFmtSpecifierToken()
{
	ASSERT(*ts == ';');
	ts++;

	while (ts < te && (*ts == ' ' || *ts == '\t'))
		ts++;

	return ts < te ? createStringToken(TokenKind_FmtSpecifier) : NULL;
}

Token*
Lexer::createDoxyCommentToken(TokenKind tokenKind)
{
	if (!(m_channelMask & TokenChannelMask_DoxyComment))
		return NULL;

	ASSERT(te - ts >= 3 && *ts == '/');
	size_t suffix = 0;

	if (tokenKind >= TokenKind_DoxyComment3) // multiline c-style: /** or /*!
	{
		ASSERT(ts[1] == '*' && te[-1] == '/' && te[-2] == '*');
		suffix = 2;
	}

	Token* token = createStringToken(tokenKind, 3, suffix);
	token->m_channelMask = TokenChannelMask_DoxyComment;
	return token;
}

bool
Lexer::onLeftCurlyBrace()
{
	if (m_mode != LexerMode_Parse)
	{
		createToken('{');
		return false;
	}

	ASSERT(m_curlyBraceLevel == 0);
	m_bodyToken = preCreateToken(TokenKind_Body);
	m_curlyBraceLevel = 1;
	return true;
}

bool
Lexer::onRightCurlyBrace()
{
	ASSERT(m_mode == LexerMode_Parse && m_bodyToken == m_tokenList.getTail().p() && m_curlyBraceLevel);

	if (--m_curlyBraceLevel)
		return false;

	m_bodyToken->m_pos.m_length = te - m_bodyToken->m_pos.m_p;
	m_bodyToken->m_data.m_string = sl::StringRef(m_bodyToken->m_pos.m_p, m_bodyToken->m_pos.m_length);
	return true;
}

void
Lexer::onLeftParentheses()
{
	if (!m_parenthesesLevelStack.isEmpty())
		m_parenthesesLevelStack[m_parenthesesLevelStack.getCount() - 1]++;

	createToken('(');
}

bool
Lexer::onRightParentheses()
{
	if (!m_parenthesesLevelStack.isEmpty())
	{
		size_t i = m_parenthesesLevelStack.getCount() - 1;
		if (m_parenthesesLevelStack[i] == 1)
		{
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
Lexer::onSemicolon()
{
	ASSERT(*ts == ';');

	if (!m_parenthesesLevelStack.isEmpty())
	{
		size_t i = m_parenthesesLevelStack.getCount() - 1;
		if (m_parenthesesLevelStack[i] == 1)
		{
			p = ts - 1; // need to reparse semicolon with 'fmt_spec' machine
			return false;
		}
	}

	createToken(';');
	return true;
}

void
Lexer::terminateFmtSpecifier()
{
	ASSERT(!m_parenthesesLevelStack.isEmpty() && m_fmtLiteralToken == NULL);
	m_parenthesesLevelStack.pop();
	preCreateFmtLiteralToken();
}

//..............................................................................

} // namespace ct
} // namespace jnc
