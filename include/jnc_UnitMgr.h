// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_AttributeMgr.h"

namespace jnc {

class CModule;

//.............................................................................

class CUnit: public rtl::TListLink
{
	friend class CUnitMgr;

protected:
	CModule* m_pModule;

	rtl::CString m_FilePath;
	rtl::CString m_FileName;
	rtl::CString m_Dir;

	llvm::DIFile m_LlvmDiFile;

public:
	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	rtl::CString
	GetFilePath ()
	{
		return m_FilePath;
	}

	rtl::CString
	GetFileName ()
	{
		return m_FileName;
	}

	rtl::CString
	GetDir ()
	{
		return m_Dir;
	}

	llvm::DIFile
	GetLlvmDiFile ()
	{
		return m_LlvmDiFile;
	}
};

//.............................................................................

class CUnitMgr
{
protected:
	CModule* m_pModule;
	rtl::CStdListT <CUnit> m_UnitList;
	CUnit* m_pCurrentUnit;

public:
	CUnitMgr ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	void
	Clear ();

	CUnit*
	GetCurrentUnit ()
	{
		return m_pCurrentUnit;
	}

	CUnit*
	SetCurrentUnit (CUnit* pUnit);

	CUnit*
	CreateUnit (const rtl::CString& FilePath);
};

//.............................................................................

} //namespace jnc {
