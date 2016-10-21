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
#include "test_ast.h"
#include "MainFrm.h"
#include "DasmPane.h"
#include "AstDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//..............................................................................

BEGIN_MESSAGE_MAP(CDasmPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CDasmPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY(m_LogCtrl.Create(
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
		ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST));

	m_LogCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_LogCtrl.SetFont (&theApp.m_Font);

	EnableDocking(CBRS_ALIGN_ANY);

	return 0;
}

void CDasmPane::OnSize(UINT nType, int cx, int cy)
{
	m_LogCtrl.MoveWindow(0, 0, cx, cy);
}

bool
CDasmPane::Build (jnc::CModule* pModule)
{
	jnc::CDisassembler Dasm;

	rtl::CIteratorT <jnc::CFunction> Function = pModule->m_FunctionMgr.GetFunctionList ().GetHead ();
	for (; Function; Function++)
	{
		jnc::CFunctionType* pFunctionType = Function->GetType ();

		m_LogCtrl.Trace (
			"%s %s %s %s\r\n",
			pFunctionType->GetReturnType ()->GetTypeString (),
			pFunctionType->GetCallConv ()->GetCallConvString (),
			Function->m_Tag,
			pFunctionType->GetArgString ()
			);

		void* pf = Function->GetMachineCode ();
		size_t Size = Function->GetMachineCodeSize ();

		if (pf)
		{
			rtl::CString s = Dasm.Disassemble (pf, Size).cc ();
			m_LogCtrl.Trace ("\r\n%s", s);
		}


		m_LogCtrl.Trace ("\r\n........................................\r\n\r\n");
	}

	return true;
}

//..............................................................................
