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

#pragma once

//..............................................................................

class CAstPane : public CDockablePane
{
protected:
	CTreeCtrl m_TreeCtrl;
	ref::CBufT<jnc::CParser::CAst> m_Ast;

public:
	bool
	Build(ref::CBufT<jnc::CParser::CAst> Ast);

	void
	Clear();

protected:
	HTREEITEM
	AddAst(
		HTREEITEM hParent,
		jnc::CParser::CAstNode* pAstNode
		);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

//..............................................................................
