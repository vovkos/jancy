#pragma once

#include "LogCtrl.h"

//..............................................................................

class CLlvmIrPane : public CDockablePane
{
protected:
	CLogCtrl m_LogCtrl;

public:
	bool
	Build (jnc::CModule* pModule);

	void
	Clear ()
	{
		m_LogCtrl.Clear ();
	}

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

//..............................................................................

