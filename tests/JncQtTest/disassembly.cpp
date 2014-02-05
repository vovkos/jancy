#include "pch.h"
#include "disassembly.h"
#include "moc_disassembly.cpp"

Disassembly::Disassembly(QWidget *parent)
	: DisassemblyBase(parent)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	setLineWrapMode (QPlainTextEdit::NoWrap);
}

bool Disassembly::build(jnc::CModule *module)
{
	clear();

	rtl::CIteratorT <jnc::CFunction> Function = module->m_FunctionMgr.GetFunctionList ().GetHead ();
	for (; Function; Function++)
	{
		addFunction (*Function);
		appendFormat ("\n;........................................\n\n");
	}

	Function = module->m_FunctionMgr.GetThunkFunctionList ().GetHead ();
	if (Function)
		appendFormat ("\n; THUNKS\n\n");

	for (; Function; Function++)
	{
		addFunction (*Function);
		appendFormat ("\n;........................................\n\n");
	}

	return true;
}

void Disassembly::addFunction(jnc::CFunction* function)
{
	jnc::CFunctionType* pFunctionType = function->GetType ();

	appendFormat (
		"%s %s %s %s\n",
		pFunctionType->GetTypeModifierString ().cc (),
		pFunctionType->GetReturnType ()->GetTypeString ().cc (),
		function->m_Tag.cc (),
		pFunctionType->GetArgString ().cc ()
		);

	void* pf = function->GetMachineCode ();
	size_t Size = function->GetMachineCodeSize ();

	if (pf)
	{
		jnc::CDisassembler Dasm;
		rtl::CString s = Dasm.Disassemble (pf, Size);
		appendFormat ("\n%s", s.cc ());
	}
}