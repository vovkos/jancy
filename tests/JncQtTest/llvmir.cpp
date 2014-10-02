#include "pch.h"
#include "llvmir.h"
#include "llvmirhighlighter.h"
#include "moc_llvmir.cpp"

LlvmIr::LlvmIr(QWidget *parent)
	: LlvmIrBase(parent)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	setLineWrapMode (QPlainTextEdit::NoWrap);
	setupHighlighter();
}

void LlvmIr::addFunction(jnc::Function* function)
{
	jnc::FunctionType* pFunctionType = function->getType ();

	appendFormat ("%s%s %s %s\n",
		pFunctionType->getTypeModifierString ().cc (),
		pFunctionType->getReturnType ()->getTypeString ().cc (),
		function->m_tag.cc (),
		pFunctionType->getArgString ().cc ()
		);

	uint_t CommentMdKind = function->getModule ()->m_llvmIrBuilder.getCommentMdKind ();

	llvm::Function* pLlvmFunction = function->getLlvmFunction ();
	llvm::Function::BasicBlockListType& BlockList = pLlvmFunction->getBasicBlockList ();
	llvm::Function::BasicBlockListType::iterator Block = BlockList.begin ();

	for (; Block != BlockList.end (); Block++)
	{
		std::string Name = Block->getName ();
		appendFormat ("%s:\n", Name.c_str ());

		llvm::BasicBlock::InstListType& InstList = Block->getInstList ();
		llvm::BasicBlock::InstListType::iterator Inst = InstList.begin ();
		for (; Inst != InstList.end (); Inst++)
		{
			std::string String;
			llvm::raw_string_ostream Stream (String);

			llvm::Instruction* pInst = Inst;

			llvm::MDNode* pMdComment = pInst->getMetadata (CommentMdKind);
			if (pMdComment)
				pInst->setMetadata (CommentMdKind, NULL); // remove before print

			pInst->print (Stream);

			llvm::DebugLoc LlvmDebugLoc = pInst->getDebugLoc ();
			if (LlvmDebugLoc.isUnknown ())
				appendFormat ("%s\n", String.c_str ());
			else
				appendFormat ("%s (line: %d)\n", String.c_str (), LlvmDebugLoc.getLine ());

			if (pMdComment)
			{
				pInst->setMetadata (CommentMdKind, pMdComment); // restore
				llvm::MDString* pMdString = (llvm::MDString*) pMdComment->getOperand (0);
				const char* pText = pMdString->getString ().data ();

				if (pText && *pText)
					appendFormat ("\n; %s\n", pText);
				else
					appendFormat ("\n", pText);
			}
		}
	}
}

bool LlvmIr::build(jnc::Module *module)
{
	clear ();

	appendText (module->getLlvmIrString ());

/*
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
	*/

	return true;
}


void LlvmIr::setupHighlighter()
{
	highlighter = new LlvmIrHighlighter(document());
}