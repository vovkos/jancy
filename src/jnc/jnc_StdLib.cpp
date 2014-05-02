#include "pch.h"
#include "jnc_StdLib.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

TIfaceHdr*
CStdLib::DynamicCastClassPtr (
	TIfaceHdr* p,
	CClassType* pType
	)
{
	if (!p)
		return NULL;

	if (p->m_pObject->m_pType->Cmp (pType) == 0)
		return p;

	CBaseTypeCoord Coord;
	bool Result = p->m_pObject->m_pClassType->FindBaseTypeTraverse (pType, &Coord);
	if (!Result)
		return NULL;

	TIfaceHdr* p2 = (TIfaceHdr*) ((uchar_t*) (p->m_pObject + 1) + Coord.m_Offset);
	ASSERT (p2->m_pObject == p->m_pObject);
	return p2;
}

TIfaceHdr*
CStdLib::StrengthenClassPtr (TIfaceHdr* p)
{
	if (!p)
		return NULL;

	EClassType ClassTypeKind = p->m_pObject->m_pClassType->GetClassTypeKind ();
	return ClassTypeKind == EClassType_FunctionClosure || ClassTypeKind == EClassType_PropertyClosure ?
		((CClosureClassType*) p->m_pObject->m_pType)->Strengthen (p) :
		(!(p->m_pObject->m_Flags & EObjHdrFlag_Dead)) ? p : NULL;
}

void*
CStdLib::GcAllocate (
	CType* pType,
	size_t ElementCount
	)
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	return pRuntime->GcAllocate (pType, ElementCount);
}

void*
CStdLib::GcTryAllocate (
	CType* pType,
	size_t ElementCount
	)
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	return pRuntime->GcTryAllocate (pType, ElementCount);
}

void
CStdLib::GcEnter ()
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);
	pRuntime->GcEnter ();
}

void
CStdLib::GcLeave ()
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	pRuntime->GcLeave ();
}

void
CStdLib::GcPulse ()
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	pRuntime->GcPulse ();
}

void
CStdLib::RunGc ()
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	pRuntime->RunGc ();
}

size_t
CStdLib::StrLen (TDataPtr Ptr)
{
	char* p = (char*) Ptr.m_p;
	if (!p)
		return 0;

	char* p0 = p;
	char* pEnd = (char*) Ptr.m_pRangeEnd;
	while (*p && p < pEnd)
		p++;

	return p - p0;
}

void
CStdLib::MemCpy (
	TDataPtr DstPtr,
	TDataPtr SrcPtr,
	size_t Size
	)
{
	if (DstPtr.m_p && SrcPtr.m_p)
		memcpy (DstPtr.m_p, SrcPtr.m_p, Size);
}

TDataPtr
CStdLib::MemCat (
	TDataPtr Ptr1,
	size_t Size1,
	TDataPtr Ptr2,
	size_t Size2
	)
{
	TDataPtr ResultPtr = { 0 };

	size_t TotalSize = Size1 + Size2;
	char* p = (char*) AXL_MEM_ALLOC (TotalSize + 1);
	if (!p)
		return ResultPtr;

	p [TotalSize] = 0; // ensure zero-termination just in case

	if (Ptr1.m_p)
		memcpy (p, Ptr1.m_p, Size1);
	else
		memset (p, 0, Size1);

	if (Ptr2.m_p)
		memcpy (p + Size1, Ptr2.m_p, Size2);
	else
		memset (p + Size1, 0, Size2);

	ResultPtr.m_p = p;
	ResultPtr.m_pRangeBegin = p;
	ResultPtr.m_pRangeEnd = p + TotalSize;
	ResultPtr.m_pObject = jnc::GetStaticObjHdr ();
	return ResultPtr;
}

#if (_AXL_ENV == AXL_ENV_WIN)

intptr_t
CStdLib::GetCurrentThreadId ()
{
	return ::GetCurrentThreadId ();
}

struct TThreadContext
{
	TFunctionPtr m_Ptr;
	CRuntime* m_pRuntime;
};

DWORD
WINAPI
CStdLib::ThreadProc (PVOID pRawContext)
{
	TThreadContext* pContext = (TThreadContext*) pRawContext;
	TFunctionPtr Ptr = pContext->m_Ptr;
	CRuntime* pRuntime = pContext->m_pRuntime;
	AXL_MEM_DELETE (pContext);

	CScopeThreadRuntime ScopeRuntime (pRuntime);
	GetTlsMgr ()->GetTls (pRuntime); // register thread right away

	((void (__cdecl*) (TIfaceHdr*)) Ptr.m_pf) (Ptr.m_pClosure);
	return 0;
}

bool
CStdLib::CreateThread (TFunctionPtr Ptr)
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	TThreadContext* pContext = AXL_MEM_NEW (TThreadContext);
	pContext->m_Ptr = Ptr;
	pContext->m_pRuntime = pRuntime;

	DWORD ThreadId;
	HANDLE h = ::CreateThread (NULL, 0, CStdLib::ThreadProc, pContext, 0, &ThreadId);
	return h != NULL;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

intptr_t
CStdLib::GetCurrentThreadId ()
{
	return (intptr_t) pthread_self ();
}

struct TThreadContext
{
	TFunctionPtr m_Ptr;
	CRuntime* m_pRuntime;
};

void*
CStdLib::ThreadProc (void* pRawContext)
{
	TThreadContext* pContext = (TThreadContext*) pRawContext;
	TFunctionPtr Ptr = pContext->m_Ptr;
	CRuntime* pRuntime = pContext->m_pRuntime;
	AXL_MEM_DELETE (pContext);

	CScopeThreadRuntime ScopeRuntime (pRuntime);
	GetTlsMgr ()->GetTls (pRuntime); // register thread right away

	((void (*) (TIfaceHdr*)) Ptr.m_pf) (Ptr.m_pClosure);
	return NULL;
}

bool
CStdLib::CreateThread (TFunctionPtr Ptr)
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	TThreadContext* pContext = AXL_MEM_NEW (TThreadContext);
	pContext->m_Ptr = Ptr;
	pContext->m_pRuntime = pRuntime;

	pthread_t Thread;
	int Result = pthread_create (&Thread, NULL, CStdLib::ThreadProc, pContext);
	return Result == 0;
}

#endif

TDataPtr
CStdLib::GetLastError ()
{
	err::CError Error = err::GetError ();
	size_t Size = Error->m_Size;

	void* p = AXL_MEM_ALLOC (Size);
	memcpy (p , Error, Size);

	jnc::TDataPtr Ptr = { 0 };
	Ptr.m_p = p;
	Ptr.m_pRangeBegin = Ptr.m_p;
	Ptr.m_pRangeEnd = (char*) Ptr.m_p + Size;
	Ptr.m_pObject = jnc::GetStaticObjHdr ();
	return Ptr;
}

TDataPtr
CStdLib::GetErrorDescription (TDataPtr Error)
{
	err::TError* pError = (err::TError*) Error.m_p;
	rtl::CString String = pError->GetDescription ();
	size_t Length = String.GetLength ();

	char* pString = (char*) AXL_MEM_ALLOC (Length + 1);
	memcpy (pString, String.cc (), Length);
	pString [Length] = 0;

	jnc::TDataPtr Ptr = { 0 };
	Ptr.m_p = pString;
	Ptr.m_pRangeBegin = Ptr.m_p;
	Ptr.m_pRangeEnd = (char*) Ptr.m_p + Length + 1;
	Ptr.m_pObject = jnc::GetStaticObjHdr ();

	return Ptr;
}

TDataPtr
CStdLib::Format (
	TDataPtr FormatString,
	...
	)
{
	AXL_VA_DECL (va, FormatString);

	char Buffer [256];
	rtl::CString String (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	String.Format_va ((const char*) FormatString.m_p, va);
	size_t Length = String.GetLength ();

	char* pString = (char*) AXL_MEM_ALLOC (Length + 1);
	memcpy (pString, String.cc (), Length);
	pString [Length] = 0;

	jnc::TDataPtr Ptr = { 0 };
	Ptr.m_p = pString;
	Ptr.m_pRangeBegin = Ptr.m_p;
	Ptr.m_pRangeEnd = (char*) Ptr.m_p + Length + 1;
	Ptr.m_pObject = jnc::GetStaticObjHdr ();

	return Ptr;
}

void*
CStdLib::GetTls ()
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	return pRuntime->GetTls () + 1;
}

size_t
CStdLib::AppendFmtLiteral_a (
	TFmtLiteral* pFmtLiteral,
	const char* p,
	size_t Length
	)
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	size_t NewLength = pFmtLiteral->m_Length + Length;
	if (NewLength < 64)
		NewLength = 64;

	if (pFmtLiteral->m_MaxLength < NewLength)
	{
		CModule* pModule = pRuntime->GetFirstModule ();
		ASSERT (pModule);

		size_t NewMaxLength = rtl::GetMinPower2Ge (NewLength);
		TObjHdr* pObjHdr = AXL_MEM_NEW_EXTRA (TObjHdr, NewMaxLength + 1);
		pObjHdr->m_ScopeLevel = 0;
		pObjHdr->m_pRoot = pObjHdr;
		pObjHdr->m_pType = pModule->m_TypeMgr.GetPrimitiveType (EType_Char);
		pObjHdr->m_Flags = 0;

		char* pNew = (char*) (pObjHdr + 1);
		memcpy (pNew, pFmtLiteral->m_p, pFmtLiteral->m_Length);

		pFmtLiteral->m_p = pNew;
		pFmtLiteral->m_MaxLength = NewMaxLength;
	}

	memcpy (pFmtLiteral->m_p + pFmtLiteral->m_Length, p, Length);
	pFmtLiteral->m_Length += Length;
	pFmtLiteral->m_p [pFmtLiteral->m_Length] = 0;

	return pFmtLiteral->m_Length;
}

void
CStdLib::PrepareFormatString (
	rtl::CString* pFormatString,
	const char* pFmtSpecifier,
	char DefaultType
	)
{
	if (!pFmtSpecifier)
	{
		char FormatBuffer [2] = { '%', DefaultType };
		pFormatString->Copy (FormatBuffer, 2);
		return;
	}

	pFormatString->Clear ();

	if (pFmtSpecifier [0] != '%')
		pFormatString->Copy ('%');

	pFormatString->Append (pFmtSpecifier);

	size_t Length = pFormatString->GetLength ();
	if (!isalpha (pFormatString->cc () [Length - 1]))
		pFormatString->Append (DefaultType);
}

size_t
CStdLib::AppendFmtLiteral_p (
	TFmtLiteral* pFmtLiteral,
	const char* pFmtSpecifier,
	TDataPtr Ptr
	)
{
	if (!Ptr.m_p)
		return AppendFmtLiteral_a (pFmtLiteral, "(null)", 6);

	char* p = (char*) Ptr.m_p;
	while (*p && p < Ptr.m_pRangeEnd)
		p++;

	if (!pFmtSpecifier || !*pFmtSpecifier)
	{
		size_t Length = p - (char*) Ptr.m_p;
		return AppendFmtLiteral_a (pFmtLiteral, (char*) Ptr.m_p, Length);
	}

	char Buffer1 [256];
	rtl::CString FormatString (ref::EBuf_Stack, Buffer1, sizeof (Buffer1));
	PrepareFormatString (&FormatString, pFmtSpecifier, 's');

	char Buffer2 [256];
	rtl::CString String (ref::EBuf_Stack, Buffer2, sizeof (Buffer2));

	if (p < Ptr.m_pRangeEnd) // null terminated
	{
		ASSERT (!*p);
		String.Format (FormatString, Ptr.m_p);
	}
	else
	{
		char Buffer3 [256];
		rtl::CString NullTermString (ref::EBuf_Stack, Buffer3, sizeof (Buffer3));
		String.Format (FormatString, NullTermString.cc ());
	}

	return AppendFmtLiteral_a (pFmtLiteral, String, String.GetLength ());
}

size_t
CStdLib::AppendFmtLiteralImpl (
	TFmtLiteral* pFmtLiteral,
	const char* pFmtSpecifier,
	char DefaultType,
	...
	)
{
	AXL_VA_DECL (va, DefaultType);

	char Buffer1 [256];
	rtl::CString FormatString (ref::EBuf_Stack, Buffer1, sizeof (Buffer1));
	PrepareFormatString (&FormatString, pFmtSpecifier,  DefaultType);

	char Buffer2 [256];
	rtl::CString String (ref::EBuf_Stack, Buffer2, sizeof (Buffer2));
	String.Format_va (FormatString, va);

	return AppendFmtLiteral_a (pFmtLiteral, String, String.GetLength ());
}

//.............................................................................

} // namespace jnc {
