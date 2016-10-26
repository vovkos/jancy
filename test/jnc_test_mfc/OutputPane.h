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

#include "LogCtrl.h"

//..............................................................................

class COutputPane : public CDockablePane
{
public:
	CLogCtrl m_LogCtrl;

protected:
	virtual
	BOOL
	PreTranslateMessage (MSG* pMsg);

	BOOL OnLButtonDblClk();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

//..............................................................................
