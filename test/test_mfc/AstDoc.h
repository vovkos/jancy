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

// AstDoc.h : interface of the CAstDoc class
//

#pragma once

class CAstDoc : public CDocument
{
protected:
	jnc::CRuntime m_Runtime;
	jnc::CModule m_Module;
	llvm::ExecutionEngine* m_pLlvmExecutionEngine;

protected: // create from serialization only
	CAstDoc();
	DECLARE_DYNCREATE(CAstDoc)

// Attributes
public:
	CEditView* GetView ()
	{
		POSITION Pos = GetFirstViewPosition ();
		return (CEditView*) GetNextView (Pos);
	}

// Operations
public:
	bool
	Compile ();

	bool
	Run ();

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CAstDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:
	bool
	RunFunction (
		jnc::CFunction* pFunction,
		int* pReturnValue = NULL
		);

	jnc::CFunction*
	FindGlobalFunction (const char* pName);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnFileCompile();
	afx_msg void OnFileRun();

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
