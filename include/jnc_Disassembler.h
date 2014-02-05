#pragma once

namespace jnc {

//.............................................................................

class CDisassembler
{
protected:
	llvm::MCDisassembler* m_pDisassembler;

public:
	CDisassembler ();

	bool
	Disassemble (
		const void* pCode,
		size_t Size,
		rtl::CString* pString
		);

	rtl::CString
	Disassemble (
		const void* pCode,
		size_t Size
		)
	{
		rtl::CString String;
		Disassemble (pCode, Size, &String);
		return String;
	}
};

//.............................................................................

} // namespace jnc {
