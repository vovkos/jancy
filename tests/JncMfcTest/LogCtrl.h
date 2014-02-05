#pragma once

//.............................................................................

class CLogCtrl: public CEdit
{
public:
	void
	Trace_0 (const char* pText);

	void
	Trace (
		const char* pFormat, 
		...
		)
	{
		AXL_VA_DECL (va, pFormat);
		Trace_va (pFormat, va);
	}
		
	void
	Trace_va (
		const char* pFormat, 
		axl_va_list va
		);

	void
	Clear ()
	{
		SetWindowText (NULL);
	}
};

//.............................................................................

