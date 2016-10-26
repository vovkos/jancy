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

// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "Resource.h"
#include "AstPane.h"
#include "ModulePane.h"
#include "LlvmIrPane.h"
#include "DasmPane.h"
#include "OutputPane.h"

class CAstDoc;

class CMainFrame : public CFrameWndEx
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CMFCRibbonBar     m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;
	CAstPane m_GlobalAstPane;
	CModulePane m_ModulePane;
	CLlvmIrPane m_LlvmIrPane;
	CDasmPane m_DasmPane;
	COutputPane m_OutputPane;

	CAstDoc* GetDocument ()
	{
		return (CAstDoc*) GetActiveDocument ();
	}


// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnClearOutput();
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	int FindFocusedOutlookWnd(CMFCOutlookBarTabCtrl** ppOutlookWnd);
};

inline
CMainFrame*
GetMainFrame ()
{
	return (CMainFrame*) AfxGetApp ()->GetMainWnd ();
}
