#pragma once

namespace jnc {

//.............................................................................

class Disassembler
{
protected:
	llvm::MCDisassembler* m_disassembler;

public:
	Disassembler ();

	bool
	disassemble (
		const void* code,
		size_t size,
		rtl::String* string
		);

	rtl::String
	disassemble (
		const void* code,
		size_t size
		)
	{
		rtl::String string;
		disassemble (code, size, &string);
		return string;
	}
};

//.............................................................................

} // namespace jnc {
