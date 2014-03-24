// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

//.............................................................................

enum EToken
{
	// common tokens

	EToken_Eof = 0,
	EToken_Error = -1,
	EToken_Identifier = 256,
	EToken_Integer,
	EToken_Fp,
	EToken_Literal,

	// special literals

	EToken_HexLiteral,
	EToken_FmtLiteral,
	EToken_FmtSpecifier,

	// global declarations & pragmas

	EToken_Namespace,
	EToken_Using,
	EToken_Extend,
	EToken_Pack,

	// storage specifiers

	EToken_Typedef,
	EToken_Alias,
	EToken_Static,
	EToken_Thread,
	EToken_Stack,
	EToken_Heap,
	EToken_UHeap,
	EToken_Abstract,
	EToken_Virtual,
	EToken_Override,
	EToken_Mutable,
	EToken_Nullable,

	// access specifiers

	EToken_Public,
	EToken_Protected,
	EToken_Friend,

	// type modifiers

	EToken_Signed,
	EToken_Unsigned,
	EToken_BigEndian,
	EToken_Const,
	EToken_DConst,
	EToken_Volatile,
	EToken_Weak,
	EToken_Thin,
	EToken_Throws,
	EToken_Cdecl,
	EToken_Stdcall,
	EToken_Array,
	EToken_Function,
	EToken_Property,
	EToken_Bindable,
	EToken_AutoGet,
	EToken_Indexed,
	EToken_Multicast,
	EToken_Event,
	EToken_DEvent,
	EToken_Reactor,

	// type specifiers

	EToken_Auto,
	EToken_Void,
	EToken_Object,
	EToken_Variant,
	EToken_Bool,
	EToken_Int8,
	EToken_Int16,
	EToken_Int32,
	EToken_Int64,
	EToken_Float,
	EToken_Double,
	EToken_Char,
	EToken_Int,
	EToken_IntPtr,

	// named type specifiers

	EToken_Enum,
	EToken_FEnum,
	EToken_CEnum,
	EToken_Struct,
	EToken_Union,
	EToken_Class,
	EToken_Opaque,

	// special member methods

	EToken_Get,
	EToken_Set,
	EToken_PreConstruct,
	EToken_Construct,
	EToken_Destruct,
	EToken_Operator,
	EToken_Postfix,

	// statements

	EToken_If,
	EToken_Else,
	EToken_For,
	EToken_While,
	EToken_Do,
	EToken_Break,
	EToken_Continue,
	EToken_Return,
	EToken_Switch,
	EToken_Case,
	EToken_Default,
	EToken_Once,
	EToken_OnEvent,
	EToken_Try,
	EToken_Catch,
	EToken_Finally,

	// pre-defined values

	EToken_BaseType,
	EToken_This,
	EToken_RetVal,
	EToken_True,
	EToken_False,
	EToken_Null,

	// keyword operators

	EToken_New,
	EToken_PNew,
	EToken_Delete,
	EToken_SizeOf,
	EToken_CountOf,
	EToken_OffsetOf,
	EToken_TypeOf,
	EToken_DTypeOf,
	EToken_BindingOf,

	// symbol tokens

	EToken_Inc,
	EToken_Dec,
	EToken_Ptr,
	EToken_Imply,
	EToken_Shl,
	EToken_Shr,
	EToken_LogAnd,
	EToken_LogOr,
	EToken_Eq,
	EToken_Ne,
	EToken_Le,
	EToken_Ge,
	EToken_RefAssign,
	EToken_AddAssign,
	EToken_SubAssign,
	EToken_MulAssign,
	EToken_DivAssign,
	EToken_ModAssign,
	EToken_ShlAssign,
	EToken_ShrAssign,
	EToken_AndAssign,
	EToken_XorAssign,
	EToken_OrAssign,
	EToken_AtAssign,
	EToken_Ellipsis,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_LEX_BEGIN_TOKEN_NAME_MAP (CTokenName)

	// common tokens

	AXL_LEX_TOKEN_NAME (EToken_Eof,          "eof")
	AXL_LEX_TOKEN_NAME (EToken_Error,        "error")
	AXL_LEX_TOKEN_NAME (EToken_Identifier,   "identifier")
	AXL_LEX_TOKEN_NAME (EToken_Integer,      "integer-constant")
	AXL_LEX_TOKEN_NAME (EToken_Fp,           "floating-point-constant")
	AXL_LEX_TOKEN_NAME (EToken_Literal,      "string-literal")
	AXL_LEX_TOKEN_NAME (EToken_HexLiteral,   "hex-literal")
	AXL_LEX_TOKEN_NAME (EToken_FmtLiteral,   "fmt-literal")
	AXL_LEX_TOKEN_NAME (EToken_FmtSpecifier, "fmt-specifier")

	// global declarations & pragmas

	AXL_LEX_TOKEN_NAME (EToken_Namespace,    "namespace")
	AXL_LEX_TOKEN_NAME (EToken_Using,        "using")
	AXL_LEX_TOKEN_NAME (EToken_Extend,       "extend")
	AXL_LEX_TOKEN_NAME (EToken_Pack,         "pack")

	// storage specifiers

	AXL_LEX_TOKEN_NAME (EToken_Typedef,      "typedef")
	AXL_LEX_TOKEN_NAME (EToken_Alias,        "alias")
	AXL_LEX_TOKEN_NAME (EToken_Static,       "static")
	AXL_LEX_TOKEN_NAME (EToken_Thread,       "thread")
	AXL_LEX_TOKEN_NAME (EToken_Stack,        "stack")
	AXL_LEX_TOKEN_NAME (EToken_Heap,         "heap")
	AXL_LEX_TOKEN_NAME (EToken_UHeap,        "uheap")
	AXL_LEX_TOKEN_NAME (EToken_Abstract,     "abstract")
	AXL_LEX_TOKEN_NAME (EToken_Virtual,      "virtual")
	AXL_LEX_TOKEN_NAME (EToken_Override,     "override")
	AXL_LEX_TOKEN_NAME (EToken_Mutable,      "mutable")
	AXL_LEX_TOKEN_NAME (EToken_Nullable,     "nullable")

	// access specifiers

	AXL_LEX_TOKEN_NAME (EToken_Public,       "public")
	AXL_LEX_TOKEN_NAME (EToken_Protected,    "protected")
	AXL_LEX_TOKEN_NAME (EToken_Friend,       "friend")

	// type modifiers

	AXL_LEX_TOKEN_NAME (EToken_Signed,       "signed")
	AXL_LEX_TOKEN_NAME (EToken_Unsigned,     "unsigned")
	AXL_LEX_TOKEN_NAME (EToken_BigEndian,    "bigendian")
	AXL_LEX_TOKEN_NAME (EToken_Const,        "const")
	AXL_LEX_TOKEN_NAME (EToken_DConst,       "dconst")
	AXL_LEX_TOKEN_NAME (EToken_Volatile,     "volatile")
	AXL_LEX_TOKEN_NAME (EToken_Weak,         "weak")
	AXL_LEX_TOKEN_NAME (EToken_Thin,         "thin")
	AXL_LEX_TOKEN_NAME (EToken_Throws,       "throws")
	AXL_LEX_TOKEN_NAME (EToken_Cdecl,        "cdecl")
	AXL_LEX_TOKEN_NAME (EToken_Stdcall,      "stdcall")
	AXL_LEX_TOKEN_NAME (EToken_Array,        "array")
	AXL_LEX_TOKEN_NAME (EToken_Function,     "function")
	AXL_LEX_TOKEN_NAME (EToken_Property,     "property")
	AXL_LEX_TOKEN_NAME (EToken_Bindable,     "bindable")
	AXL_LEX_TOKEN_NAME (EToken_AutoGet,      "autoget")
	AXL_LEX_TOKEN_NAME (EToken_Indexed,      "indexed")
	AXL_LEX_TOKEN_NAME (EToken_Multicast,    "multicast")
	AXL_LEX_TOKEN_NAME (EToken_Event,        "event")
	AXL_LEX_TOKEN_NAME (EToken_DEvent,       "devent")
	AXL_LEX_TOKEN_NAME (EToken_Reactor,      "reactor")

	// type specifiers

	AXL_LEX_TOKEN_NAME (EToken_Auto,         "auto")
	AXL_LEX_TOKEN_NAME (EToken_Void,         "void")
	AXL_LEX_TOKEN_NAME (EToken_Object,       "object")
	AXL_LEX_TOKEN_NAME (EToken_Variant,      "variant")
	AXL_LEX_TOKEN_NAME (EToken_Bool,         "bool")
	AXL_LEX_TOKEN_NAME (EToken_Int8,         "int8")
	AXL_LEX_TOKEN_NAME (EToken_Int16,        "int16")
	AXL_LEX_TOKEN_NAME (EToken_Int32,        "int32")
	AXL_LEX_TOKEN_NAME (EToken_Int64,        "int64")
	AXL_LEX_TOKEN_NAME (EToken_Float,        "float")
	AXL_LEX_TOKEN_NAME (EToken_Double,       "double")
	AXL_LEX_TOKEN_NAME (EToken_Char,         "char")
	AXL_LEX_TOKEN_NAME (EToken_Int,          "int")
	AXL_LEX_TOKEN_NAME (EToken_IntPtr,       "intptr")

	// named type specifiers

	AXL_LEX_TOKEN_NAME (EToken_Enum,         "enum")
	AXL_LEX_TOKEN_NAME (EToken_FEnum,        "fenum")
	AXL_LEX_TOKEN_NAME (EToken_CEnum,        "cenum")
	AXL_LEX_TOKEN_NAME (EToken_Struct,       "struct")
	AXL_LEX_TOKEN_NAME (EToken_Union,        "union")
	AXL_LEX_TOKEN_NAME (EToken_Class,        "class")
	AXL_LEX_TOKEN_NAME (EToken_Opaque,       "opaque")

	// special members

	AXL_LEX_TOKEN_NAME (EToken_Get,          "get")
	AXL_LEX_TOKEN_NAME (EToken_Set,          "set")
	AXL_LEX_TOKEN_NAME (EToken_PreConstruct, "preconstruct")
	AXL_LEX_TOKEN_NAME (EToken_Construct,    "construct")
	AXL_LEX_TOKEN_NAME (EToken_Destruct,     "destruct")
	AXL_LEX_TOKEN_NAME (EToken_Operator,     "operator")
	AXL_LEX_TOKEN_NAME (EToken_Postfix,      "postfix")

	// statements

	AXL_LEX_TOKEN_NAME (EToken_If,           "if")
	AXL_LEX_TOKEN_NAME (EToken_Else,         "else")
	AXL_LEX_TOKEN_NAME (EToken_For,          "for")
	AXL_LEX_TOKEN_NAME (EToken_While,        "while")
	AXL_LEX_TOKEN_NAME (EToken_Do,           "do")
	AXL_LEX_TOKEN_NAME (EToken_Break,        "break")
	AXL_LEX_TOKEN_NAME (EToken_Continue,     "continue")
	AXL_LEX_TOKEN_NAME (EToken_Return,       "return")
	AXL_LEX_TOKEN_NAME (EToken_Switch,       "switch")
	AXL_LEX_TOKEN_NAME (EToken_Case,         "case")
	AXL_LEX_TOKEN_NAME (EToken_Default,      "default")
	AXL_LEX_TOKEN_NAME (EToken_Once,         "once")
	AXL_LEX_TOKEN_NAME (EToken_OnEvent,      "onevent")
	AXL_LEX_TOKEN_NAME (EToken_Try,          "try")
	AXL_LEX_TOKEN_NAME (EToken_Catch,        "catch")
	AXL_LEX_TOKEN_NAME (EToken_Finally,      "finally")

	// pre-defined values

	AXL_LEX_TOKEN_NAME (EToken_BaseType,     "basetype")
	AXL_LEX_TOKEN_NAME (EToken_This,         "this")
	AXL_LEX_TOKEN_NAME (EToken_RetVal,       "retval")
	AXL_LEX_TOKEN_NAME (EToken_True,         "true")
	AXL_LEX_TOKEN_NAME (EToken_False,        "false")
	AXL_LEX_TOKEN_NAME (EToken_Null,         "null")

	// keyword operators

	AXL_LEX_TOKEN_NAME (EToken_New,          "new")
	AXL_LEX_TOKEN_NAME (EToken_PNew,         "pnew")
	AXL_LEX_TOKEN_NAME (EToken_Delete,       "delete")
	AXL_LEX_TOKEN_NAME (EToken_SizeOf,       "sizeof")
	AXL_LEX_TOKEN_NAME (EToken_CountOf,      "countof")
	AXL_LEX_TOKEN_NAME (EToken_OffsetOf,     "offsetof")
	AXL_LEX_TOKEN_NAME (EToken_TypeOf,       "typeof")
	AXL_LEX_TOKEN_NAME (EToken_DTypeOf,      "dtypeof")
	AXL_LEX_TOKEN_NAME (EToken_BindingOf,    "bindingof")

	// symbol tokens

	AXL_LEX_TOKEN_NAME (EToken_Inc,          "++")
	AXL_LEX_TOKEN_NAME (EToken_Dec,          "--")
	AXL_LEX_TOKEN_NAME (EToken_Ptr,          "->")
	AXL_LEX_TOKEN_NAME (EToken_Imply,        "=>")
	AXL_LEX_TOKEN_NAME (EToken_Shl,          "<<")
	AXL_LEX_TOKEN_NAME (EToken_Shr,          ">>")
	AXL_LEX_TOKEN_NAME (EToken_LogAnd,       "&&")
	AXL_LEX_TOKEN_NAME (EToken_LogOr,        "||")
	AXL_LEX_TOKEN_NAME (EToken_Eq,           "==")
	AXL_LEX_TOKEN_NAME (EToken_Ne,           "!=")
	AXL_LEX_TOKEN_NAME (EToken_Le,           "<=")
	AXL_LEX_TOKEN_NAME (EToken_Ge,           ">=")
	AXL_LEX_TOKEN_NAME (EToken_RefAssign,    ":=")
	AXL_LEX_TOKEN_NAME (EToken_AddAssign,    "+=")
	AXL_LEX_TOKEN_NAME (EToken_SubAssign,    "-=")
	AXL_LEX_TOKEN_NAME (EToken_MulAssign,    "*=")
	AXL_LEX_TOKEN_NAME (EToken_DivAssign,    "/=")
	AXL_LEX_TOKEN_NAME (EToken_ModAssign,    "%=")
	AXL_LEX_TOKEN_NAME (EToken_ShlAssign,    "<<=")
	AXL_LEX_TOKEN_NAME (EToken_ShrAssign,    ">>=")
	AXL_LEX_TOKEN_NAME (EToken_AndAssign,    "&=")
	AXL_LEX_TOKEN_NAME (EToken_XorAssign,    "^=")
	AXL_LEX_TOKEN_NAME (EToken_OrAssign,     "|=")
	AXL_LEX_TOKEN_NAME (EToken_AtAssign,     "@=")
	AXL_LEX_TOKEN_NAME (EToken_Ellipsis,     "...")

AXL_LEX_END_TOKEN_NAME_MAP ();

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CTokenData: public lex::CStdTokenData
{
public:
	rtl::CArrayT <uchar_t> m_BinData;
};

typedef lex::CRagelTokenT <EToken, CTokenName, CTokenData> CToken;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CLexer: public lex::CRagelLexerT <CLexer, CToken>
{
	friend class lex::CRagelLexerT <CLexer, CToken>;

protected:
	CToken* m_pFmtLiteralToken;
	rtl::CArrayT <intptr_t> m_ParenthesesLevelStack;

public:
	CLexer ()
	{
		m_pFmtLiteralToken = NULL;
	}

protected:
	CToken*
	CreateKeywordTokenEx (
		int Token,
		int Param
		)
	{
		CToken* pToken = CreateToken (Token);
		pToken->m_Data.m_Integer = Param;
		return pToken;
	}

	CToken*
	CreateStringToken (
		int Token,
		size_t Left = 0,
		size_t Right = 0,
		bool UseEscapeEncoding = false
		)
	{
		CToken* pToken = CreateToken (Token);
		ASSERT (pToken->m_Pos.m_Length >= Left + Right);

		size_t Length = pToken->m_Pos.m_Length - (Left + Right);
		if (UseEscapeEncoding)
			pToken->m_Data.m_String = rtl::CEscapeEncoding::Decode (ts + Left, Length);
		else
			pToken->m_Data.m_String.Copy (ts + Left, Length);

		return pToken;
	}

	CToken*
	CreateHexLiteralToken ()
	{
		CToken* pToken = CreateToken (EToken_HexLiteral);
		ASSERT (pToken->m_Pos.m_Length >= 4);
		pToken->m_Data.m_BinData = rtl::CHexEncoding::Decode (ts + 3, pToken->m_Pos.m_Length - 4);
		return pToken;
	}

	CToken*
	CreateCharToken (int Token)
	{
		CToken* pToken = CreateToken (Token);
		pToken->m_Data.m_Integer = ts [1];
		return pToken;
	}

	CToken*
	CreateIntegerToken (
		int Radix = 10,
		size_t Left = 0
		)
	{
		CToken* pToken = CreateToken (EToken_Integer);
		pToken->m_Data.m_Int64_u = _strtoui64 (ts + Left, NULL, Radix);
		return pToken;
	}

	CToken*
	CreateFpToken ()
	{
		CToken* pToken = CreateToken (EToken_Fp);
		pToken->m_Data.m_Double = strtod (ts, NULL);
		return pToken;
	}

	CToken*
	CreateConstIntegerToken (int Value)
	{
		CToken* pToken = CreateToken (EToken_Integer);
		pToken->m_Data.m_Integer = Value;
		return pToken;
	}

	// formatting literals

	CToken*
	PreCreateFmtLiteralToken ()
	{
		ASSERT (!m_pFmtLiteralToken);
		m_pFmtLiteralToken = PreCreateToken (0);
		return m_pFmtLiteralToken;
	}

	CToken*
	CreateFmtLiteralToken (int Token)
	{
		ASSERT (m_pFmtLiteralToken);
		CToken* pToken = m_pFmtLiteralToken;

		size_t Left = pToken->m_Pos.m_Length;
		size_t Right = te - ts;

		m_pFmtLiteralToken = NULL;

		pToken->m_Pos.m_Length = te - pToken->m_Pos.m_p;
		ASSERT (pToken->m_Pos.m_Length >= Left + Right);

		pToken->m_Token = Token;
		pToken->m_Data.m_String = rtl::CEscapeEncoding::Decode (
			pToken->m_Pos.m_p + Left,
			pToken->m_Pos.m_Length - (Left + Right));
		return pToken;
	}

	CToken*
	CreateFmtSimpleIdentifierToken ()
	{
		CreateFmtLiteralToken (EToken_FmtLiteral);

		// important: prevent Break () -- otherwise we could feed half-created fmt-literal token to the parser

		size_t PrevTokenizeLimit = m_TokenizeLimit;
		m_TokenizeLimit = -1;

		CToken* pToken = CreateStringToken (EToken_Identifier, 1, 0);

		m_TokenizeLimit = PrevTokenizeLimit;

		PreCreateFmtLiteralToken ();
		return pToken;
	}

	CToken*
	CreateFmtSpecifierToken ()
	{
		ASSERT (*ts == ',');
		ts++;

		while (ts < te && (*ts == ' ' || *ts == '\t'))
			ts++;

		return ts < te ? CreateStringToken (EToken_FmtSpecifier) : NULL;
	}

	void
	OnLeftParentheses ()
	{
		if (!m_ParenthesesLevelStack.IsEmpty ())
			m_ParenthesesLevelStack [m_ParenthesesLevelStack.GetCount () - 1]++;

		CreateToken ('(');
	}

	bool
	OnRightParentheses ()
	{
		if (!m_ParenthesesLevelStack.IsEmpty ())
		{
			size_t i = m_ParenthesesLevelStack.GetCount () - 1;
			if (m_ParenthesesLevelStack [i] == 1)
			{
				m_ParenthesesLevelStack.Pop ();
				PreCreateFmtLiteralToken ();
				return false;
			}

			m_ParenthesesLevelStack [i]--;
		}

		CreateToken (')');
		return true;
	}

	bool
	OnComma ()
	{
		if (!m_ParenthesesLevelStack.IsEmpty ())
		{
			size_t i = m_ParenthesesLevelStack.GetCount () - 1;
			if (m_ParenthesesLevelStack [i] == 1)
			{
				ASSERT (*ts == ',');
				p = ts - 1; // need to reparse colon with 'fmt_spec' machine
				return false;
			}
		}

		CreateToken (',');
		return true;
	}

	void
	TerminateFmtSpecifier ()
	{
		ASSERT (!m_ParenthesesLevelStack.IsEmpty () && m_pFmtLiteralToken == NULL);
		m_ParenthesesLevelStack.Pop ();
		PreCreateFmtLiteralToken ();
	}

	void
	TerminateFmtLiteral ()
	{
		ASSERT (!m_ParenthesesLevelStack.IsEmpty () && m_pFmtLiteralToken == NULL);
		m_ParenthesesLevelStack.Pop ();
	}

	// implemented in *.rl

	void
	Init ();

	void
	Exec ();
};

//.............................................................................

} // namespace jnc {
