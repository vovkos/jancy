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

void LlvmIr::addFunction(jnc::CFunction* function)
{
	jnc::CFunctionType* pFunctionType = function->GetType ();

	appendFormat ("%s%s %s %s\n",
		pFunctionType->GetTypeModifierString ().cc (),
		pFunctionType->GetReturnType ()->GetTypeString ().cc (),
		function->m_Tag.cc (),
		pFunctionType->GetArgString ().cc ()
		);

	uint_t CommentMdKind = function->GetModule ()->m_LlvmIrBuilder.GetCommentMdKind ();

	llvm::Function* pLlvmFunction = function->GetLlvmFunction ();
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

bool LlvmIr::build(jnc::CModule *module)
{
	clear ();

	appendText (module->GetLlvmIrString ());

/*
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
	*/

	return true;
}


void LlvmIr::setupHighlighter()
{
	highlighter = new LlvmIrHighlighter(document());
}