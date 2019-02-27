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

class CLogCtrl: public CEdit
{
public:
	void
	Trace_0(const char* pText);

	void
	Trace(
		const char* pFormat,
		...
		)
	{
		AXL_VA_DECL(va, pFormat);
		Trace_va(pFormat, va);
	}

	void
	Trace_va(
		const char* pFormat,
		axl_va_list va
		);

	void
	Clear()
	{
		SetWindowText(NULL);
	}
};

//..............................................................................
