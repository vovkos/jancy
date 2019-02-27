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
#include "AstPane.h"
#include "AstDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//..............................................................................

BEGIN_MESSAGE_MAP(CAstPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, AFX_IDW_PANE_FIRST, OnDblClk)
	ON_NOTIFY(TVN_GETINFOTIP, AFX_IDW_PANE_FIRST, OnGetInfoTip)
END_MESSAGE_MAP()

int CAstPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY(m_TreeCtrl.Create(
		WS_CHILD | WS_VISIBLE | TVS_LINESATROOT | TVS_HASLINES | TVS_HASBUTTONS | TVS_INFOTIP,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST));

	m_TreeCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	EnableDocking(CBRS_ALIGN_ANY);

	return 0;
}

void CAstPane::OnSize(UINT nType, int cx, int cy)
{
	m_TreeCtrl.MoveWindow(0, 0, cx, cy);
}

bool
CAstPane::Build(ref::CBufT<jnc::CParser::CAst> Ast)
{
	Clear();
	m_Ast = Ast;
	AddAst(NULL, Ast->GetRoot());
	return true;
}

HTREEITEM
CAstPane::AddAst(
	HTREEITEM hParent,
	jnc::CParser::CAstNode* pAstNode
	)
{
	rtl::CString_w SymbolName = jnc::CParser::GetSymbolName(pAstNode->m_Kind);

	HTREEITEM hItem = m_TreeCtrl.InsertItem(SymbolName, hParent);
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pAstNode);

	size_t Count = pAstNode->m_Children.GetCount();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::CParser::CAstNode* pChild = pAstNode->m_Children[i];
		AddAst(hItem, pChild);
	}

	m_TreeCtrl.Expand(hItem, TVE_EXPAND);
	return hItem;
}

void
CAstPane::Clear()
{
	m_TreeCtrl.DeleteAllItems();
	m_Ast.Release();
}

void
CAstPane::OnDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
	HTREEITEM hItem = m_TreeCtrl.GetSelectedItem();
	if (!hItem)
		return;

	jnc::CParser::CAstNode* pAstNode = (jnc::CParser::CAstNode*)m_TreeCtrl.GetItemData(hItem);

	CEditView* pView = GetMainFrame()->GetDocument()->GetView();

	pView->GetEditCtrl().SetSel(
		(int)pAstNode->m_FirstToken.m_Pos.m_Offset,
		(int)pAstNode->m_LastToken.m_Pos.m_Offset + pAstNode->m_LastToken.m_Pos.m_Length
		);

	*pResult = 0;
}

void
CAstPane::OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVGETINFOTIP* pInfoTip = (NMTVGETINFOTIP*)pNMHDR;
	jnc::CParser::CAstNode* pAstNode = (jnc::CParser::CAstNode*)pInfoTip->lParam;

	rtl::CString_w NodeText(
		pAstNode->m_FirstToken.m_Pos.m_p,
		pAstNode->m_LastToken.m_Pos.m_p + pAstNode->m_LastToken.m_Pos.m_Length -
		pAstNode->m_FirstToken.m_Pos.m_p
		);

	size_t CopyLength = NodeText.GetLength();
	if (CopyLength > (size_t)pInfoTip->cchTextMax)
		CopyLength = pInfoTip->cchTextMax;

	wcsncpy(
		pInfoTip->pszText,
		NodeText,
		CopyLength
		);

	pInfoTip->pszText[CopyLength] = 0;
}
