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

#include "pch.h"
#include "test_ast.h"
#include "MainFrm.h"
#include "LlvmIrPane.h"
#include "AstDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//..............................................................................

BEGIN_MESSAGE_MAP(CLlvmIrPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CLlvmIrPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY(m_LogCtrl.Create(
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
		ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST));

	m_LogCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_LogCtrl.SetFont(&theApp.m_Font);

	EnableDocking(CBRS_ALIGN_ANY);

	return 0;
}

void CLlvmIrPane::OnSize(UINT nType, int cx, int cy)
{
	m_LogCtrl.MoveWindow(0, 0, cx, cy);
}

bool
CLlvmIrPane::Build(jnc::CModule* pModule)
{
	Clear();

	uint_t CommentMdKind = pModule->m_LlvmBuilder.GetCommentMdKind();

	rtl::CIteratorT<jnc::CFunction> Function = pModule->m_FunctionMgr.GetFunctionList().GetHead();
	for (; Function; Function++)
	{
		jnc::CFunctionType* pFunctionType = Function->GetType();

		m_LogCtrl.Trace(
			"%s %s %s %s\r\n",
			pFunctionType->GetReturnType()->GetTypeString(),
			pFunctionType->GetCallConv()->GetCallConvString(),
			Function->m_Tag,
			pFunctionType->GetArgString()
			);

		llvm::Function* pLlvmFunction = Function->GetLlvmFunction();
		llvm::Function::BasicBlockListType& BlockList = pLlvmFunction->getBasicBlockList();
		llvm::Function::BasicBlockListType::iterator Block = BlockList.begin();

		for (; Block != BlockList.end(); Block++)
		{
			std::string Name = Block->getName();
			m_LogCtrl.Trace("%s\r\n", Name.c_str ());

			llvm::BasicBlock::InstListType& InstList = Block->getInstList();
			llvm::BasicBlock::InstListType::iterator Inst = InstList.begin();
			for (; Inst != InstList.end(); Inst++)
			{
				std::string String;
				llvm::raw_string_ostream Stream(String);

				llvm::Instruction* pInst = Inst;

				llvm::MDNode* pMdComment = pInst->getMetadata(CommentMdKind);
				if (pMdComment)
					pInst->setMetadata(CommentMdKind, NULL); // remove before print

				pInst->print(Stream);

				m_LogCtrl.Trace("%s\r\n", String.c_str ());

				if (pMdComment)
				{
					pInst->setMetadata(CommentMdKind, pMdComment); // restore
					llvm::MDString* pMdString = (llvm::MDString*)pMdComment->getOperand(0);
					m_LogCtrl.Trace("\r\n; %s\r\n", pMdString->getString ().data ());
				}
			}
		}

		m_LogCtrl.Trace("\r\n........................................\r\n\r\n");
	}

	return true;
}

//..............................................................................
