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

// AstDoc.cpp : implementation of the CAstDoc class
//

#include "pch.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "test_ast.h"
#endif

#include "MainFrm.h"

#include "AstDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int
StdLib_printf (
	const char* pFormat,
	...
	)
{
	AXL_VA_DECL (va, pFormat);

	rtl::CString Text;
	Text.Format_va (pFormat, va);

	CMainFrame* pMainFrame = GetMainFrame ();
	pMainFrame->m_OutputPane.m_LogCtrl.Trace_0 (Text);

	return Text.GetLength ();
}


// CAstDoc

IMPLEMENT_DYNCREATE(CAstDoc, CDocument)

BEGIN_MESSAGE_MAP(CAstDoc, CDocument)
	ON_COMMAND(ID_FILE_COMPILE, OnFileCompile)
	ON_COMMAND(ID_FILE_RUN, OnFileRun)
END_MESSAGE_MAP()

// CAstDoc construction/destruction

CAstDoc::CAstDoc()
{
	m_pLlvmExecutionEngine = NULL;
}

CAstDoc::~CAstDoc()
{
}

void CAstDoc::OnFileCompile()
{
	Compile ();
}

void CAstDoc::OnFileRun ()
{
	Run ();
}

bool
CAstDoc::Compile ()
{
	DoFileSave ();

	bool Result;

	CMainFrame* pMainFrame = GetMainFrame ();

	if (m_pLlvmExecutionEngine)
	{
		delete m_pLlvmExecutionEngine;
		m_pLlvmExecutionEngine = NULL;
	}

	rtl::CString FilePath = GetPathName ();

	llvm::LLVMContext* pLlvmContext = new llvm::LLVMContext;
	llvm::Module* pLlvmModule = new llvm::Module (FilePath.cc (), *pLlvmContext);
	m_Module.Create (FilePath, pLlvmModule);

	llvm::EngineBuilder EngineBuilder (pLlvmModule);
	std::string ErrorString;
	EngineBuilder.setErrorStr (&ErrorString);
	EngineBuilder.setUseMCJIT(true);

	m_pLlvmExecutionEngine = EngineBuilder.create ();
	if (!m_pLlvmExecutionEngine)
	{
		pMainFrame->m_OutputPane.m_LogCtrl.Trace ("Error creating a JITter (%s)\n", ErrorString.c_str ());
		return false;
	}

	jnc::CScopeThreadModule ScopeModule (&m_Module);

	pMainFrame->m_OutputPane.m_LogCtrl.Trace ("Parsing...\n");
	pMainFrame->m_GlobalAstPane.Clear ();
	pMainFrame->m_ModulePane.Clear ();
	pMainFrame->m_LlvmIrPane.Clear ();
	pMainFrame->m_DasmPane.Clear ();

	CStringW SourceText_w;
	GetView ()->GetWindowTextW (SourceText_w);

	rtl::CString SourceText = SourceText_w;

	jnc::CLexer Lexer;
	Lexer.Create (
		FilePath,
		SourceText,
		SourceText.GetLength ()
		);

	jnc::CParser Parser;
	Parser.Create (jnc::CParser::StartSymbol, true);

	for (;;)
	{
		const jnc::CToken* pToken = Lexer.GetToken ();
		if (pToken->m_Token == jnc::EToken_Eof)
			break;

		Result = Parser.ParseToken (pToken);
		if (!Result)
		{
			rtl::CString Text = err::GetError ()->GetDescription ();
			pMainFrame->m_OutputPane.m_LogCtrl.Trace (
				"%s(%d,%d): %s\n",
				FilePath,
				pToken->m_Pos.m_Line + 1,
				pToken->m_Pos.m_Col + 1,
				Text
				);
			return false;
		}

		Lexer.NextToken ();
	}

	pMainFrame->m_OutputPane.m_LogCtrl.Trace ("Compiling...\n");
	Result = m_Module.Compile ();

	// show module contents nevetheless

	pMainFrame->m_GlobalAstPane.Build (Parser.GetAst ());
	pMainFrame->m_ModulePane.Build (&m_Module);
	pMainFrame->m_LlvmIrPane.Build (&m_Module);

	if (!Result)
	{
		pMainFrame->m_OutputPane.m_LogCtrl.Trace ("%s\n", err::GetError ()->GetDescription ());
		return false;
	}

	pMainFrame->m_OutputPane.m_LogCtrl.Trace ("JITting...\n");

	jnc::CStdLib::Export (&m_Module, m_pLlvmExecutionEngine);
	m_Module.SetFunctionPointer (m_pLlvmExecutionEngine, "printf", (void*) StdLib_printf);

	Result = m_Module.m_FunctionMgr.JitFunctions (m_pLlvmExecutionEngine);
	if (!Result)
	{
		pMainFrame->m_OutputPane.m_LogCtrl.Trace ("%s\n", err::GetError ()->GetDescription ());
		return false;
	}

	pMainFrame->m_DasmPane.Build (&m_Module);
	pMainFrame->m_OutputPane.m_LogCtrl.Trace ("Done.\n");
	return true;
}

bool
CAstDoc::RunFunction (
	jnc::CFunction* pFunction,
	int* pReturnValue
	)
{
	typedef int (*FFunction) ();
	FFunction pf = (FFunction) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		int ReturnValue = pf ();
		if (pReturnValue)
			*pReturnValue = ReturnValue;
	}
	catch (err::CError Error)
	{
		GetMainFrame ()->m_OutputPane.m_LogCtrl.Trace ("ERROR: %s\n", Error.GetDescription ());
		Result = false;
	}
	catch (...)
	{
		GetMainFrame ()->m_OutputPane.m_LogCtrl.Trace ("UNKNOWN EXCEPTION\n");
		Result = false;
	}

	return Result;
}

bool
CAstDoc::Run ()
{
	bool Result;

	CMainFrame* pMainFrame = GetMainFrame ();

	if (IsModified ())
	{
		Result = Compile ();
		if (!Result)
			return false;
	}

	jnc::CFunction* pMainFunction = FindGlobalFunction ("main");
	if (!pMainFunction)
	{
		pMainFrame->m_OutputPane.m_LogCtrl.Trace ("'main' is not found or not a function\n");
		return false;
	}

	pMainFrame->m_OutputPane.m_LogCtrl.Trace ("Running...\n");

	jnc::CScopeThreadRuntime ScopeRuntime (&m_Runtime);

	m_Runtime.Startup ();

	jnc::CFunction* pConstructor = m_Module.GetConstructor ();
	if (pConstructor)
	{
		Result = RunFunction (pConstructor);
		if (!Result)
			return false;
	}

	Result = RunFunction (pMainFunction);
	if (!Result)
		return false;

	jnc::CFunction* pDestructor = m_Module.GetDestructor ();
	if (pDestructor)
	{
		Result = RunFunction (pDestructor);
		if (!Result)
			return false;
	}

	m_Runtime.Shutdown ();

	pMainFrame->m_OutputPane.m_LogCtrl.Trace ("Done (retval = %d).\n", Result);
	return true;
}

jnc::CFunction*
CAstDoc::FindGlobalFunction (const char* pName)
{
	jnc::CModuleItem* pItem = m_Module.m_NamespaceMgr.GetGlobalNamespace ()->FindItem (pName);
	if (!pItem)
		return NULL;

	if (pItem->GetItemKind () != jnc::EModuleItem_Function)
		return NULL;

	return (jnc::CFunction*) pItem;
}

BOOL CAstDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	GetView ()->SetFont (&theApp.m_Font);

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

BOOL CAstDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	CFile File;
	BOOL Result = File.Open (lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
	if (!Result)
		return FALSE;

	size_t Size = (size_t) File.GetLength ();
	rtl::CArrayT <char> Buffer;
	Buffer.SetCount (Size + 1);
	Buffer [Size] = 0;
	File.Read (Buffer, Size);

	GetView ()->SetWindowText (rtl::CString_w (Buffer));

	m_strPathName = lpszPathName;

	Compile ();

	SetModifiedFlag(FALSE);
	return TRUE;
}

BOOL CAstDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CFile File;
	BOOL Result = File.Open (lpszPathName, CFile::modeWrite | CFile::shareDenyWrite);
	if (!Result)
		return FALSE;

	CString String_w;
	GetView ()->GetWindowText (String_w);

	rtl::CString String = String_w;
	size_t Length = String.GetLength ();

	File.Write (String, Length);
	File.SetLength (Length);

	SetModifiedFlag(FALSE);
	return TRUE;
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CAstDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = "TODO: implement thumbnail drawing here";
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CAstDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = "point;rectangle;circle;ole object;";
	SetSearchContent(strSearchContent);
}

void CAstDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CAstDoc diagnostics

#ifdef _DEBUG
void CAstDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAstDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAstDoc commands
