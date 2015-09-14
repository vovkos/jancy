#include "pch.h"
#include "jnc_Disassembler.h"

namespace jnc {

//.............................................................................

static
int
byteReader (
	uint8_t* byte,
	uint64_t offset,
	void* context
	)
{
	mem::Block* block = (mem::Block*) context;

	if (offset >= block->m_size)
		return -1;

	*byte = *((uchar_t*) block->m_p + offset);
	return 0;
}

//.............................................................................

Disassembler::Disassembler ()
{
	m_disassembler = NULL;

/*	llvm::Target* target = NULL; // get target!

	m_disassembler = target->createMCDisassembler ();



#if (_AXL_CPU == AXL_CPU_X86)
		llvm::Triple::x86,
#elif (_AXL_CPU == AXL_CPU_AMD64)
		llvm::Triple::x86_64,
#endif
		llvm::EDDisassembler::kEDAssemblySyntaxX86ATT
		// llvm::EDDisassembler::kEDAssemblySyntaxX86Intel -- does not work!
		);

	ASSERT (m_disassembler); */
}

bool
Disassembler::disassemble (
	const void* code,
	size_t size,
	rtl::String* string
	)
{
	if (!m_disassembler)
		return false;
/*
	mem::Block block ((void*) code, size);

	size_t address = 0;
	while (address < size)
	{
		llvm::EDInst* inst = m_disassembler->createInst (byteReader, address, &block);
		if (!inst)
			break;

		size_t tokenCount = inst->numTokens ();
		if (tokenCount == -1)
			return false;

		string->appendFormat ("%08x  ", address);

		for (size_t i = 0; i < tokenCount; i++)
		{
			llvm::EDToken* token;
			inst->getToken (token, i);

			const char* tokenString;
			token->getString (tokenString);

			string->append (tokenString);
		}

		string->append ("\n");

		address += (size_t) inst->byteSize ();
		delete inst;
	}
*/
	return true;
}

//.............................................................................

} // namespace jnc {
