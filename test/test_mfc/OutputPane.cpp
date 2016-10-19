#include "pch.h"
#include "test_ast.h"
#include "OutputPane.h"
#include "MainFrm.h"
#include "AstDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//..............................................................................

BOOL
ParseFileLineString (
	LPCTSTR buffer,
	CString* pstrFile,
	int* pnLine
	)
{
	const TCHAR* pLeft;
	const TCHAR* pRight;

	const TCHAR* p = buffer;
	for (;;)
	{
		pLeft = _tcschr(p, '(');
		if (!pLeft)
			return FALSE;

		pRight = _tcschr(pLeft, ')');
		if (!pRight)
			return FALSE;

		p = pRight + 1;

		while (isspace(*p))
			p++;

		if (*p == ':' || *p == 0)
			break;
	}

	*pstrFile = CString(buffer, (int) (pLeft - buffer));
	pstrFile->TrimLeft();
	pstrFile->TrimRight();

	if (pstrFile->IsEmpty())
		return FALSE;

	CString strLine(pLeft + 1, (int) (pRight - pLeft - 1));
	strLine.TrimLeft();
	strLine.TrimRight();

	if (strLine.IsEmpty())
		return FALSE;

	TCHAR* pEnd;
	*pnLine = _tcstol(strLine, &pEnd, 10);
	if (pEnd == (LPCTSTR) strLine)
		return FALSE;

	return TRUE;
}

//..............................................................................

BEGIN_MESSAGE_MAP(COutputPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY(m_LogCtrl.Create(
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
		ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST));

	m_LogCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_LogCtrl.SetFont (&theApp.m_Font);

	EnableDocking(CBRS_ALIGN_ANY);

	return 0;
}

void COutputPane::OnSize(UINT nType, int cx, int cy)
{
	m_LogCtrl.MoveWindow(0, 0, cx, cy);
}

BOOL COutputPane::PreTranslateMessage (MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk ();
	}

	return CDockablePane::PreTranslateMessage (pMsg);
}

BOOL COutputPane::OnLButtonDblClk ()
{
	int StartChar, StopChar;
	m_LogCtrl.GetSel (StartChar, StopChar);

	int Line = m_LogCtrl.LineFromChar (StartChar);
	TCHAR Buffer [1024] = { 0 };
	m_LogCtrl.GetLine (Line, Buffer, sizeof (Buffer) - 1);

	CString FilePath;
	BOOL Result = ParseFileLineString (Buffer, &FilePath, &Line);
	if (!Result)
		return FALSE;

	CEdit* pEditCtrl = &GetMainFrame ()->GetDocument ()->GetView ()->GetEditCtrl ();
	StartChar = pEditCtrl->LineIndex (Line - 1);
	if (StartChar == -1)
		return FALSE;

	int Length = pEditCtrl->LineLength (StartChar);
	StopChar = StartChar + Length;
	pEditCtrl->SetSel (StartChar, StopChar);
	return TRUE;
}

//..............................................................................
