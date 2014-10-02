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

bool Disassembly::build(jnc::Module *module)
{
	clear();

	rtl::Iterator <jnc::Function> Function = module->m_functionMgr.getFunctionList ().getHead ();
	for (; Function; Function++)
	{
		addFunction (*Function);
		appendFormat ("\n;........................................\n\n");
	}

	Function = module->m_functionMgr.getThunkFunctionList ().getHead ();
	if (Function)
		appendFormat ("\n; THUNKS\n\n");

	for (; Function; Function++)
	{
		addFunction (*Function);
		appendFormat ("\n;........................................\n\n");
	}

	return true;
}

void Disassembly::addFunction(jnc::Function* function)
{
	jnc::FunctionType* pFunctionType = function->getType ();

	appendFormat (
		"%s %s %s %s\n",
		pFunctionType->getTypeModifierString ().cc (),
		pFunctionType->getReturnType ()->getTypeString ().cc (),
		function->m_tag.cc (),
		pFunctionType->getArgString ().cc ()
		);

	void* pf = function->getMachineCode ();
	size_t Size = function->getMachineCodeSize ();

	if (pf)
	{
		jnc::Disassembler Dasm;
		rtl::String s = Dasm.disassemble (pf, Size);
		appendFormat ("\n%s", s.cc ());
	}
}