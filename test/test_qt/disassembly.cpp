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

	void* p = function->getMachineCode ();
	size_t size = function->getMachineCodeSize ();

	appendFormat (
		"%s %s %s %s @%x (%d bytes)\n",
		pFunctionType->getTypeModifierString ().cc (),
		pFunctionType->getReturnType ()->getTypeString ().cc (),
		function->m_tag.cc (),
		pFunctionType->getArgString ().cc (),
		p,
		size
		);

	if (p)
	{
		jnc::Disassembler dasm;
		rtl::String s = dasm.disassemble (p, size);
		appendFormat ("\n%s", s.cc ());
	}
}