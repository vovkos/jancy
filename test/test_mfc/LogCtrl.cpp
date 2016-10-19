#include "pch.h"
#include "test_ast.h"
#include "OutputPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//..............................................................................

void
CLogCtrl::Trace_va (
	const char* pFormat,
	axl_va_list va
	)
{
	rtl::CString Text;
	Text.Format_va (pFormat, va);
	Trace_0 (Text);
}

void
CLogCtrl::Trace_0 (const char* pText)
{
	// normalize CR-LF

	const char* p0 = pText;
	const char* p  = pText;

	rtl::CString_w Text;

	for (; *p; p++)
		if (*p == '\n' && (p == p0 || *(p - 1) != '\r'))
		{
			Text.Append (p0, p - p0);
			Text.Append (L"\r\n");
			p0 = p + 1;
		}

	Text.Append (p0, p - p0);

	// add to edit control

	size_t Length = GetWindowTextLength ();
	SetSel ((int) Length, (int) Length);
	ReplaceSel (Text);
}

//..............................................................................
