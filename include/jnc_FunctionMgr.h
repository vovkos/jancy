// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_ScheduleLauncherFunction.h"
#include "jnc_ThunkFunction.h"
#include "jnc_ThunkProperty.h"
#include "jnc_PropertyTemplate.h"
#include "jnc_NamespaceMgr.h"
#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

class CFunctionMgr
{
	friend class CModule;
	friend class CDerivableType;
	friend class CClassType;
	friend class CFunction;
	friend class CParser;

protected:
	struct TEmissionContext: rtl::TListLink
	{
		CFunction* m_pCurrentFunction;

		CNamespace* m_pCurrentNamespace;
		CScope* m_pCurrentScope;
		CScopeLevelStack m_ScopeLevelStack;

		rtl::CArrayT <CBasicBlock*> m_ReturnBlockArray;
		CBasicBlock* m_pCurrentBlock;
		CBasicBlock* m_pUnreachableBlock;
		uint_t m_ControlFlowMgrFlags;

		CValue m_ThisValue;
		CValue m_ScopeLevelValue;
		rtl::CBoxListT <CValue> m_TmpStackGcRootList;

		llvm::DebugLoc m_LlvmDebugLoc;
	};

protected:
	CModule* m_pModule;

	// unfortunately LLVM does not provide a slot for back-pointer from llvm::Function, hence the map

	rtl::CHashTableMapT <llvm::Function*, CFunction*, rtl::CHashIdT <llvm::Function*> > m_LlvmFunctionMap;

	rtl::CStdListT <CFunction> m_FunctionList;
	rtl::CStdListT <CProperty> m_PropertyList;
	rtl::CStdListT <CPropertyTemplate> m_PropertyTemplateList;
	rtl::CStdListT <CScheduleLauncherFunction> m_ScheduleLauncherFunctionList;
	rtl::CStdListT <CThunkFunction> m_ThunkFunctionList;
	rtl::CStdListT <CThunkProperty> m_ThunkPropertyList;
	rtl::CStdListT <CDataThunkProperty> m_DataThunkPropertyList;
	rtl::CStdListT <CLazyStdFunction> m_LazyStdFunctionList;
	rtl::CStringHashTableMapT <CFunction*> m_ThunkFunctionMap;
	rtl::CStringHashTableMapT <CProperty*> m_ThunkPropertyMap;
	rtl::CStringHashTableMapT <CFunction*> m_ScheduleLauncherFunctionMap;

	CFunction* m_pCurrentFunction;

	CValue m_ThisValue;
	CValue m_ScopeLevelValue;

	rtl::CStdListT <TEmissionContext> m_EmissionContextStack;

	CFunction* m_StdFunctionArray [EStdFunc__Count];
	CLazyStdFunction* m_LazyStdFunctionArray [EStdFunc__Count];

public:
	CFunctionMgr ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	CFunction*
	GetCurrentFunction ()
	{
		return m_pCurrentFunction;
	}

	CFunction*
	SetCurrentFunction (CFunction* pFunction);

	CFunction*
	FindFunctionByLlvmFunction (llvm::Function* pLlvmFunction)
	{
		rtl::CHashTableMapIteratorT <llvm::Function*, CFunction*> It = m_LlvmFunctionMap.Find (pLlvmFunction);
		return It ? It->m_Value : NULL;
	}

	CProperty*
	GetCurrentProperty ()
	{
		return m_pCurrentFunction ? m_pCurrentFunction->GetProperty () : NULL;
	}

	CValue
	GetThisValue ()
	{
		return m_ThisValue;
	}

	CValue
	GetScopeLevel ()
	{
		return m_ScopeLevelValue;
	}

	void
	Clear ();

	rtl::CConstListT <CFunction>
	GetFunctionList ()
	{
		return m_FunctionList;
	}

	rtl::CConstListT <CProperty>
	GetPropertyList ()
	{
		return m_PropertyList;
	}

	rtl::CConstListT <CThunkFunction>
	GetThunkFunctionList ()
	{
		return m_ThunkFunctionList;
	}

	rtl::CConstListT <CThunkProperty>
	GetThunkPropertyList ()
	{
		return m_ThunkPropertyList;
	}

	rtl::CConstListT <CDataThunkProperty>
	GetDataThunkPropertyList ()
	{
		return m_DataThunkPropertyList;
	}

	CFunction*
	CreateFunction (
		EFunction FunctionKind,
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		const rtl::CString& Tag,
		CFunctionType* pType
		);

	CFunction*
	CreateFunction (
		EFunction FunctionKind,
		CFunctionType* pType
		)
	{
		return CreateFunction (FunctionKind, rtl::CString (), rtl::CString (), rtl::CString (), pType);
	}

	CFunction*
	CreateFunction (
		EFunction FunctionKind,
		const rtl::CString& Tag,
		CFunctionType* pType
		)
	{
		return CreateFunction (FunctionKind, rtl::CString (), rtl::CString (), Tag, pType);
	}

	CFunction*
	CreateFunction (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		CFunctionType* pType
		)
	{
		return CreateFunction (EFunction_Named, Name, QualifiedName, QualifiedName, pType);
	}

	CProperty*
	CreateProperty (
		EProperty PropertyKind,
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		const rtl::CString& Tag
		);

	CProperty*
	CreateProperty (EProperty PropertyKind)
	{
		return CreateProperty (PropertyKind, rtl::CString (), rtl::CString (), rtl::CString ());
	}

	CProperty*
	CreateProperty (
		EProperty PropertyKind,
		const rtl::CString& Tag
		)
	{
		return CreateProperty (PropertyKind, rtl::CString (), rtl::CString (), Tag);
	}

	CProperty*
	CreateProperty (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName
		)
	{
		return CreateProperty (EProperty_Normal, Name, QualifiedName, QualifiedName);
	}

	CPropertyTemplate*
	CreatePropertyTemplate ();

	bool
	Prologue (
		CFunction* pFunction,
		const CToken::CPos& Pos
		);

	bool
	Epilogue ();

	bool
	FireOnChanged ();

	void
	InternalPrologue (
		CFunction* pFunction,
		CValue* pArgValueArray = NULL,
		size_t ArgCount = 0
		);

	void
	InternalEpilogue ();

	bool
	InjectTlsPrologues ();

	bool
	JitFunctions (llvm::ExecutionEngine* pExecutionEngine);

	// std functions

	bool
	IsStdFunctionUsed (EStdFunc Func)
	{
		ASSERT (Func < EStdFunc__Count);
		return m_StdFunctionArray [Func] != NULL;
	}

	CFunction*
	GetStdFunction (EStdFunc Func);

	CLazyStdFunction*
	GetLazyStdFunction (EStdFunc Func);

	CFunction*
	GetDirectThunkFunction (
		CFunction* pTargetFunction,
		CFunctionType* pThunkFunctionType,
		bool HasUnusedClosure = false
		);

	CProperty*
	GetDirectThunkProperty (
		CProperty* pTargetProperty,
		CPropertyType* pThunkPropertyType,
		bool HasUnusedClosure = false
		);

	CProperty*
	GetDirectDataThunkProperty (
		CVariable* pTargetVariable,
		CPropertyType* pThunkPropertyType,
		bool HasUnusedClosure = false
		);

	CFunction*
	GetScheduleLauncherFunction (
		CFunctionPtrType* pTargetFunctionPtrType,
		EClassPtrType SchedulerPtrTypeKind = EClassPtrType_Normal
		);

protected:
	void
	CreateThisValue ();

	void
	PushEmissionContext ();

	void
	PopEmissionContext ();

	void
	InjectTlsPrologue (CFunction* pFunction);

	// LLVM code support functions

	CFunction*
	CreateCheckNullPtr ();

	CFunction*
	CreateCheckScopeLevel ();

	CFunction*
	CreateCheckDataPtrRange ();

	CFunction*
	CreateCheckClassPtrScopeLevel ();

	CFunction*
	CreateGetDataPtrSpan ();
};

//.............................................................................

} // namespace jnc {
